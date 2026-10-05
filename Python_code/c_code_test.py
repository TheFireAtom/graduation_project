import pandas as pd
import matplotlib.pyplot as plt

df_t = pd.read_csv("tremolo_output.csv")
df_v = pd.read_csv("vibrato_output.csv")

print(df_t.head())
print(df_t.shape)

print(df_v.head())
print(df_v.shape)

fragment_t = df_t.iloc[:22050]
fragment_v = df_v[(df_v["time"] >= 0.4) & (df_v["time"] <= 0.5)]

plt.figure(figsize=(12, 8))

plt.subplot(2, 1, 1);
plt.plot(fragment_t ['time'], fragment_t ['input'], label='Вход', alpha=0.7, color='blue')
plt.plot(fragment_t ['time'], fragment_t ['output'], label='Выход', alpha=0.7, color='orange')
plt.xlabel("Время (с)")
plt.ylabel("Амплитуда")
plt.legend()
plt.grid(True)
plt.title("Вход и выход сигнала тремоло")

plt.subplot(2, 1, 2);
plt.plot(fragment_v ['time'], fragment_v ['input'], label='Вход', alpha=0.7, color='blue')
plt.plot(fragment_v ['time'], fragment_v ['output'], label='Выход', alpha=0.7, color='orange')
plt.xlabel("Время (с)")
plt.ylabel("Амплитуда")
plt.legend()
plt.grid(True)
plt.title("Вход и выход сигнала вибрато")

plt.tight_layout()
plt.savefig("tremolo_and_vibrato.png", dpi=150)
plt.show()

