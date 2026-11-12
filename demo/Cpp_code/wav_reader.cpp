#include "wav_reader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <limits>

// Загрузка WAV-файла с поиском блоков RIFF
std::vector<float> loadWav(const std::string& filename, int& out_sampleRate) {

    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "[ERROR] Не удалось открыть файл: "
                  << filename << std::endl;
        return {};
    }

    // Определяем размер файла
    file.seekg(0, std::ios::end);

    std::streamoff fileSize = static_cast<std::streamoff>(file.tellg());

    if (fileSize < 12) {
        std::cerr << "[ERROR] Файл слишком маленький для WAV."
                  << std::endl;
        return {};
    }

    file.seekg(0, std::ios::beg);

    // Проверяем RIFF / WAVE
    char riff[4];
    uint32_t riffSize = 0;
    char wave[4];

    if (!file.read(riff, 4) ||
        !file.read(reinterpret_cast<char*>(&riffSize), 4) ||
        !file.read(wave, 4)) {

        std::cerr << "[ERROR] Некорректный заголовок WAV."
                  << std::endl;
        return {};
    }

    if (std::strncmp(riff, "RIFF", 4) != 0 ||
        std::strncmp(wave, "WAVE", 4) != 0) {

        std::cerr << "[ERROR] Файл не является WAV."
                  << std::endl;
        return {};
    }

    // Проверяем границы RIFF
    std::streamoff riffEnd = 8 + static_cast<std::streamoff>(riffSize);

    if (riffSize < 4 || riffEnd > fileSize) {
        std::cerr << "[ERROR] Некорректный размер RIFF."
                  << std::endl;
        return {};
    }

    // Параметры аудио
    uint16_t audioFormat = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint32_t byteRate = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;

    // Положение и размер аудиоданных
    std::streamoff dataPosition = -1;
    uint32_t dataSize = 0;

    bool fmtFound = false;
    bool dataFound = false;

    // Ищем необходимые блоки
    while (true) {

        std::streamoff currentPosition =
            static_cast<std::streamoff>(file.tellg());

        if (currentPosition < 0 || currentPosition + 8 > riffEnd) {
            break;
        }

        char chunkID[4];
        uint32_t chunkSize = 0;

        if (!file.read(chunkID, 4) ||
            !file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
            break;
        }

        std::streamoff chunkDataPosition =
            static_cast<std::streamoff>(file.tellg());

        // Проверяем границы блока
        if (chunkDataPosition < 0 ||
            chunkDataPosition > riffEnd ||
            chunkSize > riffEnd - chunkDataPosition) {

            std::cerr << "[ERROR] Размер WAV-блока выходит за границы RIFF."
                      << std::endl;
            return {};
        }

        // Блок с параметрами аудио
        if (std::strncmp(chunkID, "fmt ", 4) == 0) {

            if (chunkSize < 16) {
                std::cerr << "[ERROR] Некорректный блок fmt."
                          << std::endl;
                return {};
            }

            // Читаем поля последовательно
            if (!file.read(reinterpret_cast<char*>(&audioFormat), 2) ||
                !file.read(reinterpret_cast<char*>(&channels), 2) ||
                !file.read(reinterpret_cast<char*>(&sampleRate), 4) ||
                !file.read(reinterpret_cast<char*>(&byteRate), 4) ||
                !file.read(reinterpret_cast<char*>(&blockAlign), 2) ||
                !file.read(reinterpret_cast<char*>(&bitsPerSample), 2)) {

                std::cerr << "[ERROR] Не удалось прочитать блок fmt."
                          << std::endl;
                return {};
            }

            fmtFound = true;
        }

        // Блок с аудиоданными
        else if (std::strncmp(chunkID, "data", 4) == 0) {

            dataPosition = chunkDataPosition;
            dataSize = chunkSize;
            dataFound = true;
        }

        // Если оба блока найдены, заканчиваем поиск
        if (fmtFound && dataFound) {
            break;
        }

        // Переходим к следующему блоку
        // RIFF выравнивает блоки по 2 байта
        std::streamoff nextChunkPosition =
            chunkDataPosition +
            static_cast<std::streamoff>(chunkSize) +
            (chunkSize % 2);

        if (nextChunkPosition > riffEnd) {
            std::cerr << "[ERROR] Некорректная граница WAV-блока."
                      << std::endl;
            return {};
        }

        file.clear();
        file.seekg(nextChunkPosition, std::ios::beg);

        if (!file) {
            std::cerr << "[ERROR] Ошибка перемещения по WAV-файлу."
                      << std::endl;
            return {};
        }
    }

    // Проверяем наличие необходимых блоков
    if (!fmtFound || !dataFound) {
        std::cerr << "[ERROR] Не найдены блоки fmt или data."
                  << std::endl;
        return {};
    }

    // Проверяем формат аудио
    if (audioFormat != 1) {
        std::cerr << "[ERROR] Поддерживается только PCM WAV."
                  << std::endl;
        return {};
    }

    // Проверяем количество каналов
    if (channels != 1 && channels != 2) {
        std::cerr << "[ERROR] Поддерживается только моно и стерео."
                  << std::endl;
        return {};
    }

    // Проверяем битность
    if (bitsPerSample != 16) {
        std::cerr << "[ERROR] Поддерживается только 16-bit PCM."
                  << std::endl;
        return {};
    }

    // Проверяем остальные параметры
    if (sampleRate == 0 ||
        blockAlign != channels * sizeof(int16_t) ||
        byteRate != sampleRate * blockAlign) {

        std::cerr << "[ERROR] Некорректные параметры аудио."
                  << std::endl;
        return {};
    }

    if (blockAlign == 0 || dataSize % blockAlign != 0) {
        std::cerr << "[ERROR] Некорректный размер аудиоданных."
                  << std::endl;
        return {};
    }

    // Записываем частоту дискретизации
    out_sampleRate = static_cast<int>(sampleRate);

    // Количество аудиокадров (сэмплов на канал)
    size_t numSamples = dataSize / blockAlign;

    std::vector<float> audioData;
    audioData.reserve(numSamples);

    // Переходим к аудиоданным
    file.clear();
    file.seekg(dataPosition, std::ios::beg);

    if (!file) {
        std::cerr << "[ERROR] Не удалось перейти к аудиоданным."
                  << std::endl;
        return {};
    }

    // Читаем аудиоданные
    for (size_t i = 0; i < numSamples; ++i) {

        int16_t leftChannel = 0;
        int16_t rightChannel = 0;

        // Читаем левый канал
        if (!file.read(
                reinterpret_cast<char*>(&leftChannel),
                sizeof(int16_t))) {

            std::cerr << "[ERROR] Ошибка чтения аудиоданных."
                      << std::endl;
            return {};
        }

        float left = leftChannel / 32768.0f;

        // Если стерео — читаем правый канал
        if (channels == 2) {

            if (!file.read(
                    reinterpret_cast<char*>(&rightChannel),
                    sizeof(int16_t))) {

                std::cerr << "[ERROR] Ошибка чтения правого канала."
                          << std::endl;
                return {};
            }

            float right = rightChannel / 32768.0f;

            // Смешиваем каналы в моно
            audioData.push_back((left + right) * 0.5f);
            //audioData.push_back(left);
        }
        else {
            audioData.push_back(left);
        }
    }

    // Информация о загруженном аудио
    std::cout << "\n=== WAV INFO ===" << std::endl;

    std::cout << "Sample rate: "
              << sampleRate << " Hz" << std::endl;

    std::cout << "Channels: "
              << channels << std::endl;

    std::cout << "Bits per sample: "
              << bitsPerSample << std::endl;

    std::cout << "Samples: "
              << numSamples << std::endl;

    std::cout << "Duration: "
              << static_cast<double>(numSamples) / sampleRate
              << " sec" << std::endl;

    std::cout << "================\n" << std::endl;

    return audioData;
}


// Сохранение WAV-файла
void saveWav(
    const std::string& filename,
    const std::vector<float>& audioData,
    int sampleRate
) {

    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "[ERROR] Не удалось создать файл: "
                  << filename << std::endl;
        return;
    }

    // Проверяем, помещаются ли данные в размер RIFF
    if (audioData.size() >
        (std::numeric_limits<uint32_t>::max() - 36) / sizeof(int16_t)) {

        std::cerr << "[ERROR] Слишком большой WAV-файл."
                  << std::endl;
        return;
    }

    if (sampleRate <= 0) {
        std::cerr << "[ERROR] Некорректная частота дискретизации."
                  << std::endl;
        return;
    }

    // Создаём и инициализируем заголовок
    WAV_HEADER header{};

    uint32_t dataSize =
        static_cast<uint32_t>(audioData.size() * sizeof(int16_t));

    // RIFF
    std::memcpy(header.RIFF, "RIFF", 4);
    header.ChunkSize = 36 + dataSize;
    std::memcpy(header.WAVE, "WAVE", 4);

    // fmt
    std::memcpy(header.fmt, "fmt ", 4);
    header.Subchunk1Size = 16;
    header.AudioFormat = 1;
    header.NumOfChan = 1;
    header.SamplesPerSec = sampleRate;
    header.bytesPerSec = sampleRate * 2;
    header.blockAlign = 2;
    header.bitsPerSample = 16;

    // data
    std::memcpy(header.Subchunk2ID, "data", 4);
    header.Subchunk2Size = dataSize;

    // Записываем заголовок
    file.write(
        reinterpret_cast<const char*>(&header),
        sizeof(WAV_HEADER)
    );

    // Записываем аудиоданные
    for (float sample : audioData) {

        float clamped = std::max(-1.0f, std::min(1.0f, sample));

        int16_t pcmSample =
            static_cast<int16_t>(clamped * 32767.0f);

        file.write(
            reinterpret_cast<const char*>(&pcmSample),
            sizeof(int16_t)
        );
    }

    if (!file) {
        std::cerr << "[ERROR] Ошибка при записи WAV-файла."
                  << std::endl;
        return;
    }

    std::cout << "[OK] WAV-файл сохранён: "
              << filename << std::endl;
}
