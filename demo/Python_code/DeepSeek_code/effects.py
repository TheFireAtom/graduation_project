import numpy as np

def tremolo(data, fs, rate, depth):
    t = np.arange(len(data)) / fs
    lfo = 1 + depth * np.sin(2 * np.pi * rate * t)
    lfo = lfo[:, np.newaxis]
    output = data * lfo
    return output