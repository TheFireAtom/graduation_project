#include <stdio.h>
#include <math.h>

#define FS 44100
#define	DURATION 1.0
#define	LENGTH (int)(FS * DURATION)
#define PI 3.141592653589793f

float input[LENGTH];
float output[LENGTH];

void inputSignal() {
	for (int i = 0; i < LENGTH; i++) {
		input[i] = sinf(2.0f * PI * 440.0f * (float)i / FS);
		output[i] = input[i];
	}
}

void tremolo(float* data, int len, float fs, float rate, float depth) {
	for (int i = 0; i < len; i++) {
		float t = (float)i / fs;
		float lfo = 1.0f + depth * sinf(2.0f * PI * rate * t);
		data[i] *= lfo;
	}
}

int main() {
	inputSignal();
	tremolo(output, LENGTH, FS, 5.0f, 0.5f);

	FILE* f = fopen("output.csv", "w");

	if (f == NULL) {
		printf("File cannot be opened. Try again\n");
		return 1;
	} else {
		fprintf(f, "time,input,output\n");
		for (int i = 0; i < LENGTH; i++) {
			fprintf(f, "%f,%f,%f\n", (float)i / FS, input[i], output[i]);
		}
		fclose(f);
	}

	printf("Done. File saved\n");

	return 0;
}