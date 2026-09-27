import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf
from effects import Tremolo

# Data from out audio file and other info (time = 0.1 seconds)

data, fs = sf.read("clean-guitar-riff_E_minor.wav")

rate = 0.5
depth = 0.5

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
plt.plot(t[start:start+length], output[start:start+length, 0], label="Output magnitude plot", color="red", alpha=0.6)
plt.title("Input and output signals")
plt.xlabel("Time")
plt.ylabel("Magnitude")
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(t[start:start+length], lfo[start:start+length], label="LFO", color="purple", alpha=0.7)
plt.title("LFO signal")
plt.xlabel("Time")
plt.ylabel("Magnitude")
plt.grid(True)

plt.tight_layout()
plt.show()

# Second plot

# plt.plot(np.fft.rfftfreq())