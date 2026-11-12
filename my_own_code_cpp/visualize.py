
    
  
from pathlib import Path
import sys
import wave

import matplotlib.pyplot as plt
import numpy as np

# Библиотека для воспроизведения звука необязательна для построения графиков.
# Если она не установлена, программа всё равно сможет открыть графики.
try:
    import sounddevice as sd
except ImportError:
    sd = None


# ------------------------------------------------------------
# ЧТЕНИЕ WAV-ФАЙЛОВ ДЛЯ ВОСПРОИЗВЕДЕНИЯ
# ------------------------------------------------------------

def read_wav(filename: Path):
    """Читает WAV PCM 16 бит и возвращает отсчёты и частоту дискретизации."""
    with wave.open(str(filename), "rb") as wav_file:
        channels = wav_file.getnchannels()
        sample_rate = wav_file.getframerate()
        sample_width = wav_file.getsampwidth()
        frame_count = wav_file.getnframes()
        raw_data = wav_file.readframes(frame_count)

    if sample_width != 2:
        raise ValueError("Для воспроизведения поддерживается только PCM 16 бит.")

    # Преобразуем последовательность байтов в знаковые 16-битные числа.
    audio = np.frombuffer(raw_data, dtype="<i2").astype(np.float32)

    # Нормализуем значения к диапазону примерно от -1 до 1.
    audio /= 32768.0

    # Для стереофайла разделяем чередующиеся отсчёты на два канала.
    if channels > 1:
        audio = audio.reshape(-1, channels)

    return audio, sample_rate


# ------------------------------------------------------------
# ВОСПРОИЗВЕДЕНИЕ СОХРАНЁННЫХ WAV-ФАЙЛОВ
# ------------------------------------------------------------

def play_wav_files(output_directory: Path):
    """Последовательно воспроизводит оригинал, Tremolo и Vibrato."""
    if sd is None:
        print(
            "[ПРЕДУПРЕЖДЕНИЕ] Библиотека sounddevice не установлена. "
            "Воспроизведение пропущено."
        )
        return

    wav_files = [
        output_directory / "output_original.wav",
        output_directory / "output_tremolo.wav",
        output_directory / "output_vibrato.wav",
    ]

    for filename in wav_files:
        if not filename.exists():
            print(f"[ПРЕДУПРЕЖДЕНИЕ] Файл не найден: {filename}")
            continue

        try:
            audio, sample_rate = read_wav(filename)

            print(f"Воспроизведение: {filename.name}")
            sd.play(audio, sample_rate)
            sd.wait()

        except Exception as error:
            print(f"[ОШИБКА] Не удалось воспроизвести {filename.name}: {error}")
            sd.stop()


# ------------------------------------------------------------
# ПОСТРОЕНИЕ ГРАФИКОВ ИЗ CSV-ФАЙЛОВ
# ------------------------------------------------------------

def plot_csv(filename: Path):
    """Строит графики исходного и обработанного сигналов из CSV-файла."""
    if not filename.exists():
        print(f"[ПРЕДУПРЕЖДЕНИЕ] CSV-файл не найден: {filename}")
        return

    # Читаем таблицу, используя первую строку в качестве названий столбцов.
    data = np.genfromtxt(filename, delimiter=",", names=True, encoding="utf-8")
    data = np.atleast_1d(data)

    if data.size == 0 or data.dtype.names is None:
        print(f"[ПРЕДУПРЕЖДЕНИЕ] В CSV-файле нет данных: {filename}")

