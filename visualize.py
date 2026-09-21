import numpy as np
import matplotlib.pyplot as plt 
import soundfile as sf
from effects import tremolo

# Находим lfo на 0.1 секунде 

data, fs = sf.read("clean-guitar-riff_E_minor.wav")

rate, depth = 0.3, 0.9

output = tremolo(data, fs, rate=rate, depth=depth)

t = np.arange(len(data)) / fs

lfo = 1 + depth * np.sin(2 * np.pi * t)

start = 0
length = int(0.1 * fs)

# Стороим сам график

plt.figure(figsize=(12, 8))

plt.subplot(2, 1, 1)
plt.plot(t[start:start+length], data[start:start+length, 0], label='Вход', alpha=0.7)
plt.plot(t[start:start+length], output[start:start+length, 0], label='Выход', alpha=0.7)
plt.xlabel('Время (с)')
plt.ylabel('Амплитуда')
plt.legend()
plt.title('Вход и выход (тремоло)')
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(t[start:start+length], lfo[start:start+length], color='red', linestyle='--')
plt.xlabel('Время (с)')
plt.ylabel('LFO')
plt.title('Управляющий сигнал LFO')
plt.grid(True)

plt.tight_layout()
plt.show()
