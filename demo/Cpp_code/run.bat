
@echo off
chcp 65001 > nul
cd /d "%~dp0"

echo ========================================
echo       GUITAR EFFECTS SIMULATOR
echo ========================================

echo.
echo === 1. Компиляция C++ ===

set "PATH=C:\Portable_apps\w64devkit\bin;%PATH%"

g++ main.cpp wav_reader.cpp -o game.exe

if errorlevel 1 (
    echo [ERROR] Ошибка компиляции!
    pause
    exit /b 1
)

echo [OK] Компиляция завершена.

echo.
echo === 2. Обработка звука ===

game.exe

if errorlevel 1 (
    echo [ERROR] Ошибка выполнения C++ программы!
    pause
    exit /b 1
)

echo [OK] Обработка звука завершена.

echo.
echo === 3. Воспроизведение и построение графиков ===

where python > nul 2>&1

if errorlevel 1 (
    py -3 plot_graphs.py
) else (
    python plot_graphs.py
)

if errorlevel 1 (
    echo [ERROR] Ошибка Python!
    pause
    exit /b 1
)

echo.
echo ========================================
echo             [OK] ГОТОВО!
echo ========================================

pause
