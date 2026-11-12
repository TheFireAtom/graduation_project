@echo off
setlocal

rem Переключаем консоль Windows на UTF-8, чтобы русские сообщения отображались корректно.
chcp 65001 >nul

rem Переходим в папку, где находится этот BAT-файл.
pushd "%~dp0"

rem Получаем имя входного WAV-файла и папки результатов из аргументов запуска.
rem Если аргументы не переданы, используем значения по умолчанию.
set "INPUT_FILE=%~1"
if not defined INPUT_FILE set "INPUT_FILE=input.wav"

set "OUTPUT_DIR=%~2"
if not defined OUTPUT_DIR set "OUTPUT_DIR=output"

rem Проверяем наличие компилятора C++.
where g++ >nul 2>nul
if errorlevel 1 (
    echo [ОШИБКА] Компилятор g++ не найден. Установите MinGW-w64 и добавьте его в PATH.
    goto :error
)

rem Проверяем наличие Python.
where python >nul 2>nul
if errorlevel 1 (
    echo [ОШИБКА] Python не найден. Установите Python и добавьте его в PATH.
    goto :error
)

rem Проверяем наличие исходного WAV-файла.
if not exist "%INPUT_FILE%" (
    echo [ОШИБКА] Не найден входной файл: %INPUT_FILE%
    goto :error
)

rem Создаём папку для скомпилированной программы, если её ещё нет.
if not exist build mkdir build

rem Компилируем главную программу и модуль чтения/записи WAV.
echo.
echo ===== КОМПИЛЯЦИЯ C++ =====
g++ -std=c++17 -O2 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=UTF-8 main.cpp wav_reader.cpp -o build\audio_project.exe
if errorlevel 1 (
    echo [ОШИБКА] Не удалось скомпилировать программу.
    goto :error
)

rem Запускаем программу обработки аудио и передаём ей входной файл и папку результатов.
echo.
echo ===== ОБРАБОТКА АУДИО =====
build\audio_project.exe "%INPUT_FILE%" "%OUTPUT_DIR%"
if errorlevel 1 (
    echo [ОШИБКА] Программа обработки аудио завершилась с ошибкой.
    goto :error
)

rem Запускаем Python-скрипт для воспроизведения WAV и построения графиков.
echo.
echo ===== ВОСПРОИЗВЕДЕНИЕ И ГРАФИКИ =====
python visualize.py "%OUTPUT_DIR%"
if errorlevel 1 (
    echo [ОШИБКА] Python-скрипт завершился с ошибкой.
    goto :error
)

echo.
echo ===== ВСЕ ЭТАПЫ УСПЕШНО ЗАВЕРШЕНЫ =====
popd
pause
exit /b 0

:error
popd
pause
exit /b 1
