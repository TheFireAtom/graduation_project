import numpy as np
from abc import ABC, abstractmethod

# Абстрактный класс Effect от которого всё наследуется

class Effect(ABC):
    @abstractmethod

    def process(self, data, fs):
        pass

# Дочерние классы

class Tremolo(Effect):

    def __init__(self, rate=5.0, depth=5.0):
        self.rate = rate
        self.depth = depth

    def process(self, data, fs):
        t = np.arange(len(data)) / fs
        lfo = 1 + self.depth * np.sin(2 * np.pi * self.rate * t)
        lfo = lfo[:, np.newaxis]
        output = data * lfo
        return output