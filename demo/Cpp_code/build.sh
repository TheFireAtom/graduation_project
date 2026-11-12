#!/bin/sh
echo "=== 1. Компиляция C++ ==="
g++ main.cpp wav_reader.cpp -o game.exe
if [ $? -ne 0 ]; then
    echo "Ошибка компиляции!"
    exit 1
fi

echo "=== 2. Запуск обработки звука ==="
./game.exe

echo "=== 3. Построение графиков ==="
# ЗАМЕНИ "../python" на реальное имя папки, где лежит plot_graphs.py
python ../python/plot_graphs.py
