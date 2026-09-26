import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf
from effects import Tremolo

# Data from out audio file and other info (time = 0.1 seconds)

data, fs = sf.read("clean-guitar-riff_E_minor.wav")

rate = 5
depth = 0.9

tr = Tremolo(rate, depth)
output = tr.process(data, fs)

t = np.arange(len(data)) / fs

lfo = 1 + depth * np.sin(2 * np.pi * rate * t)

start = 0
length = int(0.1 * fs)

# Out first two plots: Input and Output signals

plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(t[start:start+length], data[start:start+length, 0], label="Input magnitude plot", color="blue", alpha=0.7)
plt.title("Input signal plot")
plt.xlabel("Time")
plt.ylabel("Magnitude")
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(t[start:start+length], output[start:start+length, 0], label="Output magnitude plot", color="red", alpha=0.7)
plt.title("Output signal plot")
plt.xlabel("Time")
plt.ylabel("Magnitude")
plt.grid(True)

plt.tight_layout()
plt.show()