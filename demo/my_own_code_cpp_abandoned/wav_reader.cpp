#include "wav_reader.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>

namespace {

// Считывает из потока ровно заданное количество байтов.
bool readBytes(std::istream& file, char* destination, std::streamsize count) {
    return static_cast<bool>(file.read(destination, count));
}

// Считывает 16-битное беззнаковое число в порядке байтов Little Endian.
bool readU16LE(std::istream& file, std::uint16_t& value) {
    unsigned char bytes[2]{};

    if (!readBytes(file, reinterpret_cast<char*>(bytes), 2)) {
        return false;
    }

    value = static_cast<std::uint16_t>(bytes[0]) |
            (static_cast<std::uint16_t>(bytes[1]) << 8);

    return true;
}

// Считывает 32-битное беззнаковое число в порядке байтов Little Endian.
bool readU32LE(std::istream& file, std::uint32_t& value) {
    unsigned char bytes[4]{};

    if (!readBytes(file, reinterpret_cast<char*>(bytes), 4)) {
        return false;
    }

    value = static_cast<std::uint32_t>(bytes[0]) |
            (static_cast<std::uint32_t>(bytes[1]) << 8) |
            (static_cast<std::uint32_t>(bytes[2]) << 16) |
            (static_cast<std::uint32_t>(bytes[3]) << 24);

    return true;
}

// Записывает 16-битное число в порядке байтов Little Endian.
bool writeU16LE(std::ostream& file, std::uint16_t value) {
    const char bytes[2] = {
        static_cast<char>(value & 0xFFu),
        static_cast<char>((value >> 8) & 0xFFu)
    };

    return static_cast<bool>(file.write(bytes, 2));
}

// Записывает 32-битное число в порядке байтов Little Endian.
bool writeU32LE(std::ostream& file, std::uint32_t value) {
    const char bytes[4] = {
        static_cast<char>(value & 0xFFu),
        static_cast<char>((value >> 8) & 0xFFu),
        static_cast<char>((value >> 16) & 0xFFu),
        static_cast<char>((value >> 24) & 0xFFu)
    };

    return static_cast<bool>(file.write(bytes, 4));
}

// Записывает четырёхсимвольный идентификатор RIFF-чанка.
bool writeChunkId(std::ostream& file, const char (&id)[5]) {
    return static_cast<bool>(file.write(id, 4));
}

// Записывает сообщение об ошибке, не требуя от вызывающей функции
// самостоятельно формировать текст для каждого случая.
bool fail(std::string& error, const std::string& message) {
    error = message;
    return false;
}

} // конец закрытого пространства имён

std::size_t WavData::frameCount() const noexcept {
    if (channels == 0) {
        return 0;
    }

    return samples.size() / channels;
}

bool loadWav(
    const std::filesystem::path& filename,
    WavData& result,
    std::string& error
) {
    result = WavData{};
    error.clear();

    // Открываем файл в бинарном режиме, чтобы читать исходные байты без преобразований.
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return fail(error, "Не удалось открыть файл: " + filename.string());
    }

    // Получаем физический размер файла для проверки границ RIFF-чанков.
    std::error_code fileSizeError;
    const std::uintmax_t physicalFileSize =
        std::filesystem::file_size(filename, fileSizeError);

    if (fileSizeError) {
        return fail(error, "Не удалось определить размер WAV-файла.");
    }

    if (physicalFileSize < 12) {
        return fail(error, "Файл слишком мал и не содержит полноценный заголовок WAV.");
    }

    // Проверяем идентификаторы RIFF и WAVE.
    char riffId[4]{};
    char waveId[4]{};
    std::uint32_t riffSize = 0;

    if (!readBytes(file, riffId, 4) || !readU32LE(file, riffSize) ||
        !readBytes(file, waveId, 4)) {
        return fail(error, "Не удалось прочитать заголовок WAV.");
    }

    if (std::memcmp(riffId, "RIFF", 4) != 0) {
        return fail(error, "Файл не является RIFF-контейнером.");
    }

    if (std::memcmp(waveId, "WAVE", 4) != 0) {
        return fail(error, "В RIFF-контейнере отсутствует формат WAVE.");
    }

    // Размер RIFF включает байты после первых восьми байтов файла.
    const std::uint64_t riffEnd = 8ull + riffSize;

    if (riffSize < 4 || riffEnd > physicalFileSize) {
        return fail(error, "Размер RIFF-области не соответствует размеру файла.");
    }

    bool formatFound = false;
    bool dataFound = false;

    std::uint16_t audioFormat = 0;
    std::uint16_t channels = 0;
    std::uint16_t blockAlign = 0;
    std::uint16_t bitsPerSample = 0;
    std::uint32_t sampleRate = 0;
    std::uint32_t byteRate = 0;
    std::uint32_t dataSize = 0;
    std::uint64_t dataOffset = 0;

    // Ищем чанки fmt и data по всему RIFF-контейнеру.
    // Такой подход не предполагает, что аудиоданные начинаются строго с 44-го байта.
    while (true) {
        const std::streampos currentPosition = file.tellg();

        if (currentPosition < 0) {
            return fail(error, "Не удалось определить позицию внутри WAV-файла.");
        }

        const std::uint64_t currentOffset =
            static_cast<std::uint64_t>(currentPosition);

        if (currentOffset == riffEnd) {
            break;
        }

        if (currentOffset > riffEnd || riffEnd - currentOffset < 8) {
            return fail(error, "Повреждена структура RIFF-чанков.");
        }

        char chunkId[4]{};
        std::uint32_t chunkSize = 0;

        if (!readBytes(file, chunkId, 4) || !readU32LE(file, chunkSize)) {
            return fail(error, "Не удалось прочитать заголовок чанка.");
        }

        const std::streampos payloadPosition = file.tellg();

        if (payloadPosition < 0) {
            return fail(error, "Не удалось определить начало данных чанка.");
        }

        const std::uint64_t payloadOffset =
            static_cast<std::uint64_t>(payloadPosition);

        // В RIFF после чанка нечётной длины добавляется байт выравнивания.
        const std::uint64_t paddedChunkSize =
            static_cast<std::uint64_t>(chunkSize) + (chunkSize % 2u);

        const std::uint64_t nextChunkOffset = payloadOffset + paddedChunkSize;

        if (nextChunkOffset < payloadOffset || nextChunkOffset > riffEnd ||
            nextChunkOffset > physicalFileSize) {
            return fail(error, "Размер одного из чанков выходит за границы файла.");
        }

        if (std::memcmp(chunkId, "fmt ", 4) == 0) {
            if (formatFound) {
                return fail(error, "В файле обнаружено несколько fmt-чанков; такой вариант не поддерживается.");
            }

            // Первые 16 байтов fmt-чанка содержат обязательные параметры PCM.
            if (chunkSize < 16) {
                return fail(error, "Чанк fmt слишком короткий.");
            }

            if (!readU16LE(file, audioFormat) ||
                !readU16LE(file, channels) ||
                !readU32LE(file, sampleRate) ||
                !readU32LE(file, byteRate) ||
                !readU16LE(file, blockAlign) ||
                !readU16LE(file, bitsPerSample)) {
                return fail(error, "Не удалось прочитать параметры аудиоформата.");
            }

            formatFound = true;
        }
        else if (std::memcmp(chunkId, "data", 4) == 0) {
            if (dataFound) {
                return fail(error, "В файле обнаружено несколько data-чанков; такой вариант не поддерживается.");
            }

            dataOffset = payloadOffset;
            dataSize = chunkSize;
            dataFound = true;
        }

        // Переходим к следующему чанку, пропуская неиспользуемые данные и байт выравнивания.
        file.seekg(static_cast<std::streamoff>(nextChunkOffset), std::ios::beg);

        if (!file) {
            return fail(error, "Не удалось перейти к следующему RIFF-чанку.");
        }
    }

    if (!formatFound) {
        return fail(error, "В WAV-файле отсутствует чанк fmt.");
    }

    if (!dataFound) {
        return fail(error, "В WAV-файле отсутствует чанк data.");
    }

    // Проверяем, что файл содержит обычный несжатый PCM с 16-битными отсчётами.
    if (audioFormat != 1) {
        return fail(error, "Поддерживается только несжатый PCM WAV.");
    }

    if (channels != 1 && channels != 2) {
        return fail(error, "Поддерживаются только моно- и стереофайлы.");
    }

    if (bitsPerSample != 16) {
        return fail(error, "Поддерживаются только WAV-файлы с разрядностью 16 бит.");
    }

    if (sampleRate == 0) {
        return fail(error, "Частота дискретизации не может быть равна нулю.");
    }

    if (sampleRate > static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
        return fail(error, "Частота дискретизации слишком велика для обработки этой программой.");
    }

    const std::uint16_t expectedBlockAlign =
        static_cast<std::uint16_t>(channels * (bitsPerSample / 8));

    const std::uint64_t expectedByteRate =
        static_cast<std::uint64_t>(sampleRate) * expectedBlockAlign;

    if (blockAlign != expectedBlockAlign || byteRate != expectedByteRate) {
        return fail(error, "Параметры byte rate или block align противоречат формату WAV.");
    }

    if (dataSize % blockAlign != 0) {
        return fail(error, "Размер аудиоданных не кратен размеру одного аудиокадра.");
    }

    // Возвращаемся к началу аудиоданных и преобразуем каждый 16-битный отсчёт в float.
    file.seekg(static_cast<std::streamoff>(dataOffset), std::ios::beg);

    if (!file) {
        return fail(error, "Не удалось перейти к началу аудиоданных.");
    }

    const std::size_t sampleCount = static_cast<std::size_t>(dataSize / 2u);
    std::vector<float> decodedSamples;
    decodedSamples.reserve(sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        std::uint16_t rawSample = 0;

        if (!readU16LE(file, rawSample)) {
            return fail(error, "Не удалось полностью прочитать аудиоданные.");
        }

        // Преобразуем беззнаковое представление байтов в знаковый диапазон int16.
        const std::int32_t signedSample =
            (rawSample < 0x8000u)
                ? static_cast<std::int32_t>(rawSample)
                : static_cast<std::int32_t>(rawSample) - 65536;

        // Нормализуем отсчёт: деление на 32768 соответствует формату PCM 16 бит.
        decodedSamples.push_back(static_cast<float>(signedSample) / 32768.0f);
    }

    result.sampleRate = sampleRate;
    result.channels = channels;
    result.bitsPerSample = bitsPerSample;
    result.samples = std::move(decodedSamples);

    return true;
}

bool saveWav(
    const std::filesystem::path& filename,
    const WavData& format,
    const std::vector<float>& samples,
    std::string& error
) {
    error.clear();

    // Проверяем параметры аудиоформата и согласованность количества каналов.
    if (format.sampleRate == 0) {
        return fail(error, "Нельзя сохранить WAV с нулевой частотой дискретизации.");
    }

    if (format.channels != 1 && format.channels != 2) {
        return fail(error, "Для сохранения поддерживаются один или два канала.");
    }

    if (format.bitsPerSample != 16) {
        return fail(error, "Для сохранения поддерживается только PCM 16 бит.");
    }

    if (samples.size() % format.channels != 0) {
        return fail(error, "Количество отсчётов не соответствует числу каналов.");
    }

    // Проверяем все значения до создания файла, чтобы не оставить частично записанный WAV.
    for (float sample : samples) {
        if (!std::isfinite(sample)) {
            return fail(error, "В аудиоданных обнаружено некорректное значение NaN или бесконечность.");
        }
    }

    // Размер PCM-данных в байтах: каждый отсчёт занимает два байта.
    const std::uint64_t dataSize64 =
        static_cast<std::uint64_t>(samples.size()) * 2ull;

    // Стандартный RIFF WAV с заголовком PCM занимает 36 байт до аудиоданных.
    if (dataSize64 > std::numeric_limits<std::uint32_t>::max() - 36ull) {
        return fail(error, "Слишком большой файл для стандартного RIFF WAV.");
    }

    const std::uint16_t blockAlign =
        static_cast<std::uint16_t>(format.channels * 2u);

    const std::uint64_t byteRate64 =
        static_cast<std::uint64_t>(format.sampleRate) * blockAlign;

    if (byteRate64 > std::numeric_limits<std::uint32_t>::max()) {
        return fail(error, "Параметр byte rate превышает допустимый размер.");
    }

    const auto dataSize = static_cast<std::uint32_t>(dataSize64);
    const auto byteRate = static_cast<std::uint32_t>(byteRate64);
    const std::uint32_t riffSize = 36u + dataSize;

    // Открываем выходной файл в бинарном режиме.
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return fail(error, "Не удалось создать WAV-файл: " + filename.string());
    }

    // Записываем основной заголовок RIFF/WAVE.
    bool writeSuccess =
        writeChunkId(file, "RIFF") &&
        writeU32LE(file, riffSize) &&
        writeChunkId(file, "WAVE");

    // Записываем чанк fmt с параметрами несжатого PCM.
    writeSuccess = writeSuccess &&
        writeChunkId(file, "fmt ") &&
        writeU32LE(file, 16u) &&                 // Размер данных чанка fmt.
        writeU16LE(file, 1u) &&                  // 1 означает несжатый PCM.
        writeU16LE(file, format.channels) &&
        writeU32LE(file, format.sampleRate) &&
        writeU32LE(file, byteRate) &&
        writeU16LE(file, blockAlign) &&
        writeU16LE(file, 16u);                   // Разрядность одного отсчёта.

    // Записываем заголовок чанка data.
    writeSuccess = writeSuccess &&
        writeChunkId(file, "data") &&
        writeU32LE(file, dataSize);

    if (!writeSuccess) {
        return fail(error, "Не удалось записать заголовок WAV-файла.");
    }

    // Преобразуем нормализованные значения float обратно в PCM 16 бит.
    for (float sample : samples) {
        // Ограничиваем амплитуду допустимым диапазоном.
        const float clippedSample = std::clamp(sample, -1.0f, 1.0f);

        // Масштабируем значение и ограничиваем его диапазоном знакового int16.
        long quantized = std::lround(clippedSample * 32768.0f);
        quantized = std::clamp(quantized, -32768l, 32767l);

        // Записываем битовое представление отсчёта в порядке Little Endian.
        const auto pcmSample = static_cast<std::uint16_t>(
            static_cast<std::int16_t>(quantized)
        );

        if (!writeU16LE(file, pcmSample)) {
            return fail(error, "Ошибка во время записи аудиоданных WAV-файла.");
        }
    }

    file.flush();

    if (!file.good()) {
        return fail(error, "Не удалось завершить запись WAV-файла.");
    }

    return true;
}
