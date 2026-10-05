#include <stdio.h>
#include <math.h>

#define FS 44100
#define	DURATION 1
#define	LENGTH (FS * DURATION)
#define PI 3.141592653589793f
#define MAX_DELAY_SECONDS 14
#define MAX_DELAY_SAMPLES ((int)(FS * MAX_DELAY_SECONDS / 1000) + 2)

float tremolo_input[LENGTH];
float tremolo_output[LENGTH];
float buffer[MAX_DELAY_SAMPLES];
float vibrato_input[LENGTH];
float vibrato_output[LENGTH];

int write_index = 0;

void inputSignal() {
	for (int i = 0; i < LENGTH; i++) {
		tremolo_input[i] = sinf(2.0f * PI * 440.0f * (float)i / FS);
		vibrato_input[i] = sinf(2.0f * PI * 440.0f * (float)i / FS);
		tremolo_output[i] = tremolo_input[i];
		vibrato_output[i] = vibrato_input[i];
	}
}

void tremolo(float* data, int len, float fs, float rate, float depth) {
	for (int i = 0; i < len; i++) {
		float t = (float)i / fs;
		float lfo = 1.0f + depth * sinf(2.0f * PI * rate * t);
		data[i] *= lfo;
	}
}

void vibrato(float* input, float* output, int len, float fs, float rate, float depth) {
	float depth_samples = depth * fs / 1000.0f;
	float base_delay = 10 * fs / 1000;

	for (int i = 0; i < len; i++) {
		buffer[write_index] = input[i];

		if (i < base_delay + depth_samples) {
			output[i] = input[i];
		} else {
			float lfo = sinf(2.0f * PI * rate * (float)i/fs);
			float delay_samples = base_delay + depth_samples * lfo;

			float read_index = write_index - delay_samples;
			read_index = fmodf(read_index + MAX_DELAY_SAMPLES, MAX_DELAY_SAMPLES);
			if (read_index < 0) {
				read_index += MAX_DELAY_SAMPLES;
			}

			int i_0 = (int)floorf(read_index);
			int i_1 = (i_0 + 1) % MAX_DELAY_SAMPLES;
			float frac = read_index - i_0;
			
			output[i] = buffer[i_0] * (1.0f - frac) + buffer[i_1] * frac;

		}

		write_index = (write_index + 1) % MAX_DELAY_SAMPLES;
	}
}

int main() {
	inputSignal();
	tremolo(tremolo_output, LENGTH, FS, 5.0f, 0.5f);
	vibrato(vibrato_input, vibrato_output, LENGTH, FS, 5.0f, 3.0f);

	FILE* t = fopen("tremolo_output.csv", "w");
	FILE* v = fopen("vibrato_output.csv", "w");

	if (t == NULL) {
		printf("File cannot be opened. Try again\n");
		return 1;
	} else {
		fprintf(t, "time,input,output\n");
		for (int i = 0; i < LENGTH; i++) {
			fprintf(t, "%f,%f,%f\n", (float)i / FS, tremolo_input[i], tremolo_output[i]);
		}
		fclose(t);
	}

	if (v == NULL) {
		printf("File cannot be opened. Try again\n");
		return 1;
	} else {
		fprintf(v, "time,input,output\n");
		for (int i = 0; i < LENGTH; i++) {
			fprintf(v, "%f,%f,%f\n", (float)i / FS, vibrato_input[i], vibrato_output[i]);
		}
		fclose(v);
	}

	printf("Done. File saved\n");

	return 0;
}