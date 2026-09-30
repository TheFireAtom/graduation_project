import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("output.csv")

print(df.head())
print(df.shape)

fragment = df.iloc[:22050]

plt.figure(figsize=(12, 8))

#plt.subplot(2, 1, 1);
plt.plot(fragment['time'], fragment['input'], label='Вход', alpha=0.7, color='blue')
plt.plot(fragment['time'], fragment['output'], label='Выход', alpha=0.7, color='orange')
plt.xlabel("Время (с)")
plt.ylabel("Амплитуда")
plt.legend()
plt.grid(True)
plt.title("Вход и выход сигнала тремоло")

plt.tight_layout()
plt.show()

plt.savefig("tremolo.png", dpi=150)