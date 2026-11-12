#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include "wav_reader.h"
#include <windows.h>

// #define FS 44100
// #define	DURATION 1
// #define	LENGTH (FS * DURATION)
#define PI 3.141592653589793f
// #define MAX_DELAY_SECONDS 14
// #define MAX_DELAY_SAMPLES ((int)(FS * MAX_DELAY_SECONDS / 1000) + 2)

// Для генерации тестового сигнала внутри кода
// float tremolo_input[LENGTH];
// float tremolo_output[LENGTH];
// float buffer[MAX_DELAY_SAMPLES];
// float vibrato_input[LENGTH];
// float vibrato_output[LENGTH];

int write_index = 0;

// Для генерации тестового сигнала внутри кода
// void inputSignal() {
// 	for (int i = 0; i < LENGTH; i++) {
// 		tremolo_input[i] = sinf(2.0f * PI * 440.0f * (float)i / FS);
// 		vibrato_input[i] = sinf(2.0f * PI * 440.0f * (float)i / FS);
// 		tremolo_output[i] = tremolo_input[i];
// 		vibrato_output[i] = vibrato_input[i];
// 	}
// }

void tremolo(std::vector<float>& audioData, int sampleRate, float rate, float depth) {
	for (int i = 0; i < audioData.size(); i++) {
		float t = (float)i / sampleRate;
		float lfo = 1.0f + depth * sinf(2.0f * PI * rate * t);
		audioData[i] *= lfo;
	}
}

void vibrato(std::vector<float>& audioData, float sampleRate, float rate, float depth_ms) {
	int maxDelay = (int)(sampleRate * 0.02); // Максимальная задержка буфера (20 мс)
	std::vector<float> buffer(maxDelay, 0.0f);

	int writeIndex = 0;
	float baseDelay = 0.005f * sampleRate; // Базовая задержка (5 мс)
	float depthSamples = depth_ms * sampleRate / 1000.0f; // "Глубина" изменения звука

	for (size_t i = 0; i < audioData.size(); i++) {
		buffer[writeIndex] = audioData[i];
		float t = float(i) / sampleRate;
		float lfo = sinf(2.0f * PI * rate * t); // Low frequency oscillator, низкочастотный сигнал - сердце вибрато

		float delaySamples = baseDelay + depthSamples * lfo;
		float readIndex = writeIndex - delaySamples;

		while (readIndex < 0) readIndex += maxDelay;
		while (readIndex >= maxDelay) readIndex -= maxDelay;

		int i0 = (int)floorf(readIndex);
		int i1 = (i0 + 1) % maxDelay;
		float frac = readIndex - i0;
		audioData[i] = buffer[i0] * (1.0f - frac) + buffer[i1] * frac; 

		writeIndex = (writeIndex + 1) % maxDelay;

	}
}

// Эта функция должна быть ВЫШЕ функции main()
void saveCsv(const std::string& filename, const std::vector<float>& input, const std::vector<float>& output, int sampleRate) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось создать файл " << filename << std::endl;
        return;
    }
    
    file << "time,input,output\n";
    for (size_t i = 0; i < input.size(); ++i) {
        file << (float)i / sampleRate << "," << input[i] << "," << output[i] << "\n";
    }
    file.close();
}

int main() {
 	// Set console input and output code page to UTF-8 (65001) or CP1251 (1251)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // Set C++ locale
    // setlocale(LC_ALL, "Russian");

    int sampleRate = 44100;
    std::vector<float> originalAudio;

    // 1. Пытаемся загрузить твой сэмпл гитары
    std::cout << "Поиск файла input.wav..." << std::endl;
    originalAudio = loadWav("input.wav", sampleRate);

    std::cout << "\n=== AUDIO DIAGNOSTICS ===" << std::endl;

    std::cout << "Sample count: "
              << originalAudio.size() << std::endl;

    std::cout << "Sample rate: "
              << sampleRate << " Hz" << std::endl;

    std::cout << "Duration: "
              << static_cast<double>(originalAudio.size()) / sampleRate
              << " sec" << std::endl;

    std::cout << "========================\n" << std::endl;

    std::cout << "Поиск файла input.wav..." << std::endl;

    originalAudio = loadWav("input.wav", sampleRate);

    if (originalAudio.empty()) {
        std::cerr << "[ERROR] Не удалось загрузить input.wav."
                  << std::endl;
        return 1;
    }

    std::cout << "[OK] Гитарная запись успешно загружена."
              << std::endl;

    // 3. Делаем копии оригинального сигнала для каждого эффекта
    // (важно, чтобы эффекты не накладывались друг на друга)
    std::vector<float> tremoloAudio = originalAudio;
    std::vector<float> vibratoAudio = originalAudio;

    // 4. Применяем эффекты
    std::cout << "Применение эффекта Тремоло..." << std::endl;
    tremolo(tremoloAudio, sampleRate, 5.0f, 0.8f); // 5 Гц, глубина 80%

    std::cout << "Применение эффекта Вибрато..." << std::endl;
    vibrato(vibratoAudio, sampleRate, 5.0f, 3.0f); // 5 Гц, глубина 3 мс

    // 5. Сохраняем результаты в WAV (используя функцию из wav_reader.cpp)
    saveWav("output_tremolo.wav", tremoloAudio, sampleRate);
    saveWav("output_vibrato.wav", vibratoAudio, sampleRate);
    
    // 6. Сохраняем CSV для графиков в Python
    std::cout << "Сохранение данных для графиков..." << std::endl;
    saveCsv("tremolo_graph.csv", originalAudio, tremoloAudio, sampleRate);
    saveCsv("vibrato_graph.csv", originalAudio, vibratoAudio, sampleRate);

    std::cout << "\nУспех! Созданы WAV-файлы и CSV для графиков." << std::endl;
    return 0;
}