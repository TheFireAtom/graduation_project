#include <iostream>
#include <vector>
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

int main() {

    std::vector<float> originalAudio;
    int sampleRate = 44100;

    if (originalAudio.empty()) {

        std::cout << "Generating test signal 440 Hz..."
                  << std::endl;

        int duration = 2;
        int numSamples = sampleRate * duration;

        originalAudio.resize(numSamples);

        for (int i = 0; i < numSamples; ++i) {

            originalAudio[i] = std::sin(
                2.0f * PI * 440.0f *
                static_cast<float>(i) / sampleRate
            );
        }
    }

    std::cout << "Samples: " << originalAudio.size() << std::endl;

    std::cout << "Duration: "
              << static_cast<float>(originalAudio.size()) / sampleRate
              << " seconds" << std::endl;

    return 0;
}