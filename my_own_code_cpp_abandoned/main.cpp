#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "wav_reader.h"

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif

// Число Пи, используемое при генерации низкочастотных колебаний.
constexpr float PI = 3.14159265358979323846f;


// ============================================================
// ЭФФЕКТ ТРЕМОЛО
// ============================================================

void tremolo(
    std::vector<float>& audioData,
    int sampleRate,
    int channels,
    float rate,
    float depth
) {
    // Проверяем входные параметры и структуру аудиоданных.
    if (sampleRate <= 0 || (channels != 1 && channels != 2) ||
        audioData.size() % static_cast<std::size_t>(channels) != 0 ||
        !std::isfinite(rate) || !std::isfinite(depth) ||
        rate < 0.0f || depth < 0.0f || depth > 1.0f) {
        throw std::invalid_argument("Некорректные параметры Tremolo");
    }

    // Количество кадров учитывает число каналов.
    const std::size_t frameCount = audioData.size() / channels;

    // Обрабатываем сигнал покадрово, чтобы оба стереоканала
    // получали одинаковый коэффициент изменения амплитуды в один момент времени.
    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        // Вычисляем время текущего аудиокадра в секундах.
        const float t = static_cast<float>(frame) / sampleRate;

        // Формируем низкочастотный модулирующий сигнал (LFO).
        const float lfo = 1.0f + depth * std::sin(2.0f * PI * rate * t);

        // Применяем один и тот же коэффициент к каждому каналу кадра.
        for (int channel = 0; channel < channels; ++channel) {
            const std::size_t index = frame * channels + channel;
            audioData[index] *= lfo;
        }
    }
}


// ============================================================
// ЭФФЕКТ ВИБРАТО
// ============================================================

void vibrato(
    std::vector<float>& audioData,
    int sampleRate,
    int channels,
    float rate,
    float depthMs
) {
    // Проверяем входные параметры и структуру аудиоданных.
    if (sampleRate <= 0 || (channels != 1 && channels != 2) ||
        audioData.size() % static_cast<std::size_t>(channels) != 0 ||
        !std::isfinite(rate) || !std::isfinite(depthMs) ||
        rate < 0.0f || depthMs < 0.0f) {
        throw std::invalid_argument("Некорректные параметры Vibrato");
    }

    // Базовая задержка и максимальная длина кольцевого буфера в миллисекундах.
    constexpr float BASE_DELAY_MS = 5.0f;
    constexpr float MAX_BUFFER_MS = 20.0f;

    // Переводим длину буфера из миллисекунд в количество аудиокадров.
    const int maxDelay = static_cast<int>(
        sampleRate * MAX_BUFFER_MS / 1000.0f
    );

    if (maxDelay < 2) {
        throw std::runtime_error("Слишком маленький буфер Vibrato");
    }

    // Создаём кольцевой буфер отдельно для каждого канала.
    // Внутри вектора отсчёты расположены вперемежку по каналам.
    std::vector<float> buffer(
        static_cast<std::size_t>(maxDelay) * channels,
        0.0f
    );

    // Переводим базовую задержку и её глубину из миллисекунд в кадры.
    const float baseDelay = BASE_DELAY_MS * sampleRate / 1000.0f;
    const float depthSamples = depthMs * sampleRate / 1000.0f;

    // Проверяем, что модулируемая задержка остаётся положительной
    // и не выходит за границы кольцевого буфера.
    if (baseDelay - depthSamples < 1.0f ||
        baseDelay + depthSamples >= static_cast<float>(maxDelay - 1)) {
        throw std::invalid_argument(
            "Задержка Vibrato выходит за допустимые границы буфера"
        );
    }

    // Количество аудиокадров в исходном сигнале.
    const std::size_t frameCount = audioData.size() / channels;

    // Индекс кадра, в который будет записываться следующий отсчёт.
    int writeIndex = 0;

    // Обрабатываем аудио последовательно.
    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        // Сначала записываем исходные отсчёты текущего кадра в буфер.
        for (int channel = 0; channel < channels; ++channel) {
            const std::size_t audioIndex = frame * channels + channel;
            const std::size_t bufferIndex =
                static_cast<std::size_t>(writeIndex) * channels + channel;

            buffer[bufferIndex] = audioData[audioIndex];
        }

        // Вычисляем время текущего кадра.
        const float t = static_cast<float>(frame) / sampleRate;

        // Генерируем низкочастотный управляющий сигнал (LFO).
        const float lfo = std::sin(2.0f * PI * rate * t);

        // Вычисляем текущую задержку в аудиокадрах.
        const float delaySamples = baseDelay + depthSamples * lfo;

        // Определяем позицию чтения с учётом текущей задержки.
        float readIndex = static_cast<float>(writeIndex) - delaySamples;

        // Переводим отрицательный индекс в диапазон кольцевого буфера.
        while (readIndex < 0.0f) {
            readIndex += maxDelay;
        }

        // На всякий случай корректируем индекс, вышедший за конец буфера.
        while (readIndex >= maxDelay) {
            readIndex -= maxDelay;
        }

        // Находим два соседних кадра для линейной интерполяции.
        const int index0 = static_cast<int>(std::floor(readIndex));
        const int index1 = (index0 + 1) % maxDelay;

        // Дробная часть позиции определяет вес соседних отсчётов.
        const float fraction = readIndex - index0;

        // Выполняем интерполяцию отдельно для каждого аудиоканала.
        for (int channel = 0; channel < channels; ++channel) {
            const std::size_t bufferIndex0 =
                static_cast<std::size_t>(index0) * channels + channel;
            const std::size_t bufferIndex1 =
                static_cast<std::size_t>(index1) * channels + channel;
            const std::size_t audioIndex = frame * channels + channel;

            audioData[audioIndex] =
                buffer[bufferIndex0] * (1.0f - fraction) +
                buffer[bufferIndex1] * fraction;
        }

        // Перемещаем указатель записи на следующий кадр буфера.
        // После последнего кадра указатель возвращается в начало.
        writeIndex = (writeIndex + 1) % maxDelay;
    }
}


// ============================================================
// СОХРАНЕНИЕ ДАННЫХ ДЛЯ ПОСТРОЕНИЯ ГРАФИКОВ
// ============================================================

bool saveCsv(
    const std::filesystem::path& filename,
    const WavData& format,
    const std::vector<float>& input,
    const std::vector<float>& output,
    std::string& error
) {
    error.clear();

    // Проверяем параметры аудио и совпадение размеров исходного и обработанного сигналов.
    if (format.sampleRate == 0 ||
        (format.channels != 1 && format.channels != 2) ||
        input.size() != output.size() ||
        input.size() % format.channels != 0) {
        error = "Некорректные данные для сохранения CSV.";
        return false;
    }

    // Открываем CSV-файл для записи.
    std::ofstream file(filename);

    if (!file.is_open()) {
        error = "Не удалось создать CSV-файл: " + filename.string();
        return false;
    }

    // Устанавливаем точность записи чисел с плавающей точкой.
    file << std::setprecision(9);

    // Формируем заголовок CSV в зависимости от числа каналов.
    file << "time,input_ch1,output_ch1";

    if (format.channels == 2) {
        file << ",input_ch2,output_ch2";
    }

    file << '\n';

    // Один кадр содержит один отсчёт на каждый канал.
    const std::size_t frameCount = input.size() / format.channels;

    for (std::size_t frame = 0; frame < frameCount; ++frame) {
        // Переводим номер кадра во время в секундах.
        const double time =
            static_cast<double>(frame) / format.sampleRate;

        const std::size_t firstChannelIndex = frame * format.channels;

        // Записываем время, исходный и обработанный отсчёты первого канала.
        file << time << ','
             << input[firstChannelIndex] << ','
             << output[firstChannelIndex];

        // Для стерео дополнительно записываем отсчёты второго канала.
        if (format.channels == 2) {
            const std::size_t secondChannelIndex = firstChannelIndex + 1;

            file << ','
                 << input[secondChannelIndex] << ','
                 << output[secondChannelIndex];
        }

        file << '\n';
    }

    // Проверяем, не возникла ли ошибка во время записи.
    if (!file.good()) {
        error = "Ошибка во время записи CSV-файла: " + filename.string();
        return false;
    }

    return true;
}


// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ ПРОГРАММЫ
// ============================================================

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Устанавливаем кодовую страницу UTF-8 для вывода русских сообщений в консоль Windows.
    SetConsoleOutputCP(CP_UTF8);
#endif

    // Имя входного файла и папка для результатов могут передаваться аргументами командной строки.
    const std::filesystem::path inputFile =
        (argc >= 2) ? argv[1] : "input.wav";

    const std::filesystem::path outputDirectory =
        (argc >= 3) ? argv[2] : "output";

    // Параметры эффекта Tremolo.
    constexpr float TREMOLO_RATE = 5.0f;
    constexpr float TREMOLO_DEPTH = 0.8f;

    // Параметры эффекта Vibrato.
    constexpr float VIBRATO_RATE = 5.0f;
    constexpr float VIBRATO_DEPTH_MS = 3.0f;

    // --------------------------------------------------------
    // 1. ЗАГРУЗКА WAV-ФАЙЛА
    // --------------------------------------------------------

    WavData originalAudio;
    std::string error;

    std::cout << "Загрузка аудиофайла: " << inputFile.string() << std::endl;

    if (!loadWav(inputFile, originalAudio, error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    if (originalAudio.samples.empty()) {
        std::cerr << "[ОШИБКА] WAV-файл не содержит аудиоданных." << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 2. ВЫВОД ИНФОРМАЦИИ ОБ АУДИОЗАПИСИ
    // --------------------------------------------------------

    const double duration =
        static_cast<double>(originalAudio.frameCount()) /
        originalAudio.sampleRate;

    std::cout << "\n========== ИНФОРМАЦИЯ ОБ АУДИО ==========\n";
    std::cout << "Количество каналов: " << originalAudio.channels << '\n';
    std::cout << "Количество кадров: " << originalAudio.frameCount() << '\n';
    std::cout << "Количество отсчётов: " << originalAudio.samples.size() << '\n';
    std::cout << "Частота дискретизации: " << originalAudio.sampleRate << " Гц\n";
    std::cout << "Разрядность: " << originalAudio.bitsPerSample << " бит\n";
    std::cout << "Продолжительность: " << duration << " сек\n";
    std::cout << "=========================================\n" << std::endl;

    // --------------------------------------------------------
    // 3. СОЗДАНИЕ КОПИЙ ИСХОДНОГО СИГНАЛА
    // --------------------------------------------------------

    // Создаём независимые копии, чтобы эффекты не накладывались друг на друга.
    std::vector<float> tremoloAudio = originalAudio.samples;
    std::vector<float> vibratoAudio = originalAudio.samples;

    // --------------------------------------------------------
    // 4. ПРИМЕНЕНИЕ АУДИОЭФФЕКТОВ
    // --------------------------------------------------------

    try {
        // Применяем Tremolo к первой копии исходного сигнала.
        std::cout << "Применение эффекта Tremolo..." << std::endl;

        tremolo(
            tremoloAudio,
            static_cast<int>(originalAudio.sampleRate),
            originalAudio.channels,
            TREMOLO_RATE,
            TREMOLO_DEPTH
        );

        std::cout << "[OK] Tremolo успешно применён." << std::endl;

        // Применяем Vibrato ко второй копии исходного сигнала.
        std::cout << "Применение эффекта Vibrato..." << std::endl;

        vibrato(
            vibratoAudio,
            static_cast<int>(originalAudio.sampleRate),
            originalAudio.channels,
            VIBRATO_RATE,
            VIBRATO_DEPTH_MS
        );

        std::cout << "[OK] Vibrato успешно применён." << std::endl;
    }
    catch (const std::exception& exception) {
        // Выводим сообщение, если один из эффектов обнаружил некорректные параметры.
        std::cerr << "[ОШИБКА] Не удалось обработать аудио: "
                  << exception.what() << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 5. СОЗДАНИЕ ПАПКИ ДЛЯ РЕЗУЛЬТАТОВ
    // --------------------------------------------------------

    // Создаём папку, если она ещё не существует.
    std::error_code directoryError;
    std::filesystem::create_directories(outputDirectory, directoryError);

    if (directoryError) {
        std::cerr << "[ОШИБКА] Не удалось создать папку результатов: "
                  << directoryError.message() << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 6. СОХРАНЕНИЕ РЕЗУЛЬТАТОВ В WAV
    // --------------------------------------------------------

    std::cout << "\nСохранение WAV-файлов..." << std::endl;

    // Сохраняем исходную запись без изменений.
    if (!saveWav(
            outputDirectory / "output_original.wav",
            originalAudio,
            originalAudio.samples,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем запись после применения Tremolo.
    if (!saveWav(
            outputDirectory / "output_tremolo.wav",
            originalAudio,
            tremoloAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем запись после применения Vibrato.
    if (!saveWav(
            outputDirectory / "output_vibrato.wav",
            originalAudio,
            vibratoAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 7. СОХРАНЕНИЕ ДАННЫХ ДЛЯ ПОСТРОЕНИЯ ГРАФИКОВ
    // --------------------------------------------------------

    std::cout << "\nСохранение CSV-файлов..." << std::endl;

    // Сохраняем исходную запись: входной и выходной сигналы здесь совпадают.
    if (!saveCsv(
            outputDirectory / "original_graph.csv",
            originalAudio,
            originalAudio.samples,
            originalAudio.samples,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем исходный и обработанный сигналы Tremolo.
    if (!saveCsv(
            outputDirectory / "tremolo_graph.csv",
            originalAudio,
            originalAudio.samples,
            tremoloAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем исходный и обработанный сигналы Vibrato.
    if (!saveCsv(
            outputDirectory / "vibrato_graph.csv",
            originalAudio,
            originalAudio.samples,
            vibratoAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 8. ЗАВЕРШЕНИЕ РАБОТЫ ПРОГРАММЫ
    // --------------------------------------------------------

    std::cout << "\n=========================================\n";
    std::cout << "Обработка аудио успешно завершена!\n";
    std::cout << "Результаты сохранены в папке: "
              << outputDirectory.string() << '\n';
    std::cout << "=========================================\n";

    return 0;
}
