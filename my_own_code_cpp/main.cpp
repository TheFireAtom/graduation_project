
    
  
            originalAudio.channels,
            VIBRATO_RATE,
            VIBRATO_DEPTH_MS
        );

        std::cout << "[OK] Vibrato успешно применён." << std::endl;
    }
    catch (const std::exception& exception) {
        // Выводим сообщение, если один из эффектов обнаружил некорректные параметры.
        std::cerr << "[ОШИБКА] Не удалось обработать аудио: "
                  << exception.what() << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 5. СОЗДАНИЕ ПАПКИ ДЛЯ РЕЗУЛЬТАТОВ
    // --------------------------------------------------------

    // Создаём папку, если она ещё не существует.
    std::error_code directoryError;
    std::filesystem::create_directories(outputDirectory, directoryError);

    if (directoryError) {
        std::cerr << "[ОШИБКА] Не удалось создать папку результатов: "
                  << directoryError.message() << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 6. СОХРАНЕНИЕ РЕЗУЛЬТАТОВ В WAV
    // --------------------------------------------------------

    std::cout << "\nСохранение WAV-файлов..." << std::endl;

    // Сохраняем исходную запись без изменений.
    if (!saveWav(
            outputDirectory / "output_original.wav",
            originalAudio,
            originalAudio.samples,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем запись после применения Tremolo.
    if (!saveWav(
            outputDirectory / "output_tremolo.wav",
            originalAudio,
            tremoloAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем запись после применения Vibrato.
    if (!saveWav(
            outputDirectory / "output_vibrato.wav",
            originalAudio,
            vibratoAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 7. СОХРАНЕНИЕ ДАННЫХ ДЛЯ ПОСТРОЕНИЯ ГРАФИКОВ
    // --------------------------------------------------------

    std::cout << "\nСохранение CSV-файлов..." << std::endl;

    // Сохраняем исходную запись: входной и выходной сигналы здесь совпадают.
    if (!saveCsv(
            outputDirectory / "original_graph.csv",
            originalAudio,
            originalAudio.samples,
            originalAudio.samples,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем исходный и обработанный сигналы Tremolo.
    if (!saveCsv(
            outputDirectory / "tremolo_graph.csv",
            originalAudio,
            originalAudio.samples,
            tremoloAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // Сохраняем исходный и обработанный сигналы Vibrato.
    if (!saveCsv(
            outputDirectory / "vibrato_graph.csv",
            originalAudio,
            originalAudio.samples,
            vibratoAudio,
            error)) {
        std::cerr << "[ОШИБКА] " << error << std::endl;
        return 1;
    }

    // --------------------------------------------------------
    // 8. ЗАВЕРШЕНИЕ РАБОТЫ ПРОГРАММЫ
    // --------------------------------------------------------

    std::cout << "\n=========================================\n";
    std::cout << "Обработка аудио успешно завершена!\n";
    std::cout << "Результаты сохранены в папке: "
              << outputDirectory.string() << '\n';
    std::cout << "=========================================\n";

    return 0;
}

