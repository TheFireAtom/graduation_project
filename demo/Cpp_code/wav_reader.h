#pragma once
#include <cstdint>
#include <vector>
#include <string>

// Структура самого wav-файла 
#pragma pack(push, 1)
struct WAV_HEADER {
    char        RIFF[4];
    uint32_t    ChunkSize;
    char        WAVE[4];
    char        fmt[4];
    uint32_t    Subchunk1Size;
    uint16_t    AudioFormat;
    uint16_t    NumOfChan;
    uint32_t    SamplesPerSec;
    uint32_t    bytesPerSec;
    uint16_t    blockAlign;
    uint16_t    bitsPerSample;
    char        Subchunk2ID[4];
    uint32_t    Subchunk2Size;
};
#pragma pack(pop)

// Объявления функций 
std::vector<float> loadWav(const std::string& filename, int& out_sampleRate);
void saveWav(const std::string& filename, const std::vector<float>& audioData, int sampleRate);