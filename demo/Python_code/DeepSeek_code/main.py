import soundfile as sf
import sounddevice as sd
from DeepSeek_code.effects import tremolo

data, fs = sf.read("clean-guitar-riff_E_minor.wav")

new_sound = tremolo(data, fs, rate=0.5, depth=0.5)

sd.play(new_sound)

sd.wait()