#include <fstream>
#include <iostream> 
#include <filesystem>
#include "wavParser.hpp"

bool WavParser::load(const std::filesystem::path& filePath) {
	std::ifstream file (filePath, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Error: file cannot be opened " << std::endl;
	} else {
		std::cout << "Success: file is opened " << std::endl;
	}
}