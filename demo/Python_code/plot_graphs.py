import pandas as pd
import matplotlib.pyplot as plt
import os
import winsound  # Встроенный модуль Windows для звука

# Проверяем, есть ли файлы
if not os.path.exists("tremolo_graph.csv") or not os.path.exists("vibrato_graph.csv"):
    print("CSV файлы не найдены. Сначала запусти C++ программу!")
    exit()

# --- ВОСПРОИЗВЕДЕНИЕ ЗВУКА ---
# Проверяем наличие WAV-файлов и проигрываем их
if os.path.exists("output_tremolo.wav"):
    print("▶ Воспроизведение Тремоло...")
    # SND_FILENAME означает, что мы передаем имя файла. Скрипт подождет, пока звук доиграет.
    winsound.PlaySound("output_tremolo.wav", winsound.SND_FILENAME)
else:
    print("Файл output_tremolo.wav не найден!")

if os.path.exists("output_vibrato.wav"):
    print("▶ Воспроизведение Вибрато...")
    winsound.PlaySound("output_vibrato.wav", winsound.SND_FILENAME)
else:
    print("Файл output_vibrato.wav не найден!")

# --- ПОСТРОЕНИЕ ГРАФИКОВ ---
print("📊 Построение графиков...")
df_tremolo = pd.read_csv("tremolo_graph.csv")
df_vibrato = pd.read_csv("vibrato_graph.csv")

# Берем первые 50 миллисекунд (0.05 секунд) для наглядности
tremolo_frag = df_tremolo[df_tremolo['time'] <= 0.05]
vibrato_frag = df_vibrato[df_vibrato['time'] <= 0.05]

# Создаем окно с графиками
fig, axes = plt.subplots(2, 2, figsize=(14, 8))
fig.suptitle('Анализ аудиоэффектов (Первые 50 мс)', fontsize=16)

# --- Тремоло ---
axes[0, 0].plot(tremolo_frag['time'], tremolo_frag['input'], color='blue', label='Вход (Оригинал)')
axes[0, 0].set_title('Тремоло: Входной сигнал')
axes[0, 0].grid(True)

axes[0, 1].plot(tremolo_frag['time'], tremolo_frag['output'], color='orange', label='Выход (Тремоло)')
axes[0, 1].set_title('Тремоло: Выходной сигнал')
axes[0, 1].grid(True)

# --- Вибрато ---
axes[1, 0].plot(vibrato_frag['time'], vibrato_frag['input'], color='blue', label='Вход (Оригинал)')
axes[1, 0].set_title('Вибрато: Входной сигнал')
axes[1, 0].grid(True)

axes[1, 1].plot(vibrato_frag['time'], vibrato_frag['output'], color='green', label='Выход (Вибрато)')
axes[1, 1].set_title('Вибрато: Выходной сигнал')
axes[1, 1].grid(True)

plt.tight_layout()
plt.savefig("analysis_result.png", dpi=150)
plt.show()