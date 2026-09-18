import numpy as np
import soundfile as sf
import sounddevice as sd

data, fs = sf.read("clean-guitar-riff_E_minor.wav")

def tremolo(data, fs, rate, depth):
    t = np.arange(len(data)) / fs
    lfo = 1 + depth * np.sin(2 * np.pi * rate * t)
    lfo = lfo[:, np.newaxis]
    output = data * lfo
    return output

new_sound = tremolo(data, fs, rate=0.5, depth=0.5)

sd.play(new_sound)

sd.wait()