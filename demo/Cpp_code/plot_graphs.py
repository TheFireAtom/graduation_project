
import pandas as pd
import matplotlib.pyplot as plt
import os
import winsound
from pathlib import Path

# Рабочая директория скрипта
BASE_DIR = Path(__file__).resolve().parent

# Проверка наличия CSV
tremolo_csv = BASE_DIR / "tremolo_graph.csv"
vibrato_csv = BASE_DIR / "vibrato_graph.csv"

if not tremolo_csv.exists() or not vibrato_csv.exists():
    print("[ERROR] CSV-файлы не найдены. Сначала запусти C++ программу!")
    exit(1)


# --- ВОСПРОИЗВЕДЕНИЕ ЗВУКА ---

def play_audio(filename, effect_name):
    audio_path = BASE_DIR / filename

    if not audio_path.exists():
        print(f"[ERROR] Файл {filename} не найден!")
        return

    print(f"[P] Воспроизведение: {effect_name}")

    try:
        # Синхронное воспроизведение.
        # Программа ждёт полного окончания звука.
        winsound.PlaySound(
            str(audio_path),
            winsound.SND_FILENAME
        )

        print(f"[OK] {effect_name}: воспроизведение завершено.")

    except RuntimeError as e:
        print(f"[ERROR] Не удалось воспроизвести {filename}: {e}")


# Последовательное воспроизведение
play_audio("output_tremolo.wav", "Тремоло")
play_audio("output_vibrato.wav", "Вибрато")


# --- ПОСТРОЕНИЕ ГРАФИКОВ ---

print("\n[GRAPH] Построение графиков...")

try:
    df_tremolo = pd.read_csv(tremolo_csv)
    df_vibrato = pd.read_csv(vibrato_csv)

    # Тремоло: первая секунда
    tremolo_frag = df_tremolo[
        df_tremolo['time'] <= 1.0
    ]

    # Вибрато: окно 50 мс
    vibrato_frag = df_vibrato[
        (df_vibrato['time'] >= 0.1) &
        (df_vibrato['time'] <= 0.15)
    ]

    # Создание графиков
    fig, axes = plt.subplots(2, 2, figsize=(14, 8))

    fig.suptitle(
        'Анализ аудиоэффектов',
        fontsize=16
    )

    # --- Тремоло ---

    axes[0, 0].plot(
        tremolo_frag['time'],
        tremolo_frag['input'],
        color='blue',
        label='Входной сигнал'
    )

    axes[0, 0].set_title('Тремоло: входной сигнал (1 сек)')
    axes[0, 0].set_xlabel('Время (с)')
    axes[0, 0].set_ylabel('Амплитуда')
    axes[0, 0].grid(True)
    axes[0, 0].legend()

    axes[0, 1].plot(
        tremolo_frag['time'],
        tremolo_frag['output'],
        color='orange',
        label='Выходной сигнал'
    )

    axes[0, 1].set_title('Тремоло: выходной сигнал')
    axes[0, 1].set_xlabel('Время (с)')
    axes[0, 1].set_ylabel('Амплитуда')
    axes[0, 1].grid(True)
    axes[0, 1].legend()

    # --- Вибрато ---

    axes[1, 0].plot(
        vibrato_frag['time'],
        vibrato_frag['input'],
        color='blue',
        label='Входной сигнал'
    )

    axes[1, 0].set_title('Вибрато: входной сигнал (50 мс)')
    axes[1, 0].set_xlabel('Время (с)')
    axes[1, 0].set_ylabel('Амплитуда')
    axes[1, 0].grid(True)
    axes[1, 0].legend()

    axes[1, 1].plot(
        vibrato_frag['time'],
        vibrato_frag['output'],
        color='green',
        label='Выходной сигнал'
    )

    axes[1, 1].set_title('Вибрато: выходной сигнал')
    axes[1, 1].set_xlabel('Время (с)')
    axes[1, 1].set_ylabel('Амплитуда')
    axes[1, 1].grid(True)
    axes[1, 1].legend()

    plt.tight_layout()

    output_image = BASE_DIR / "analysis_result.png"

    plt.savefig(output_image, dpi=150)

    print(f"[OK] Графики сохранены: {output_image}")

    plt.show()

except Exception as e:
    print(f"[ERROR] Ошибка при построении графиков: {e}")
    exit(1)

print("\n=== ПРОГРАММА ЗАВЕРШЕНА ===")
