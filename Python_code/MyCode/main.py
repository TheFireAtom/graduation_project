import soundfile as sf
import sounddevice as sd
import numpy as np

from effects import Tremolo

data, fs = sf.read("clean-guitar-riff_E_minor.wav")
rate = 0.5
depth = 0.5

tremolo = Tremolo(rate, depth)

sd.play(tremolo.process(data, fs))
sd.wait()
