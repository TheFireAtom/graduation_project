#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// Структура хранит основные параметры WAV-файла и его аудиоданные.
// Отсчёты хранятся в формате float и нормализованы примерно к диапазону [-1; 1].
// Для стерео отсчёты расположены вперемежку: левый, правый, левый, правый и т. д.
struct WavData {
    std::uint32_t sampleRate = 0;       // Частота дискретизации в герцах.
    std::uint16_t channels = 0;         // Количество каналов: 1 — моно, 2 — стерео.
    std::uint16_t bitsPerSample = 16;  // Разрядность исходного WAV-файла.
    std::vector<float> samples;        // Аудиоотсчёты.

    // Возвращает количество аудиокадров.
    // Один кадр содержит по одному отсчёту каждого канала.
    std::size_t frameCount() const noexcept;
};

// Загружает WAV-файл PCM 16 бит с одним или двумя каналами.
// При ошибке возвращает false, а подробное сообщение записывает в error.
bool loadWav(
    const std::filesystem::path& filename,
    WavData& result,
    std::string& error
);

// Сохраняет переданные аудиоданные в WAV PCM 16 бит.
// Параметры частоты дискретизации и числа каналов берутся из format.
// При ошибке возвращает false, а подробное сообщение записывает в error.
bool saveWav(
    const std::filesystem::path& filename,
    const WavData& format,
    const std::vector<float>& samples,
    std::string& error
);
