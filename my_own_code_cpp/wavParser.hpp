#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

struct WavHeader {
    // Master RIFF chunk
    char riffId[4];
    std::uint32_t riffSize;
    char formType[4];

    // Chunk describing the data format
    char fmtChunkId[4];
    std::uint32_t formatChunkSize;
    std::uint16_t audioFormat;
    std::uint16_t numChannels;
    std::uint32_t sampleRate;
    std::uint32_t byteRate;
    std::uint16_t blockAlign;
    std::uint16_t bitsPerSample;

    // Chunk containing the sampled data
    char dataChunkId[4];
    std::uint32_t dataSize;
};

class WavParser {
private:
    WavHeader header{};
    std::vector<float> samples;

public:
    bool load(const std::filesystem::path& filePath);
    const WavHeader& getHeader() const;
    const std::vector<float>& getSamples() const;
};