#include "FileManager.h"
#include "utils/Utils.h" // 需要用到颜色输出
#include <iostream>
#include <fstream>
#include <string>

bool FileManager::setup_directories(const std::string& master_dir, int start_year, int end_year) {
    try {
        if (!std::filesystem::exists(master_dir)) {
            std::filesystem::create_directory(master_dir);
            std::cout << "Created master directory: '" << master_dir << "'\n";
        }
        for (int year = start_year; year <= end_year; ++year) {
            std::filesystem::path year_dir_path = std::filesystem::path(master_dir) / std::to_string(year);
            if (!std::filesystem::exists(year_dir_path)) {
                std::filesystem::create_directory(year_dir_path);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << Utils::ConsoleColors::red << "Error creating directories. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n';
        return false;
    }
    return true;
}

bool FileManager::write_log_file(const std::filesystem::path& file_path, const std::string& content) {
    std::ofstream outFile(file_path);
    if (!outFile.is_open()) {
        std::cerr << Utils::ConsoleColors::red << "Error: Could not open file '" << file_path.string() << "' for writing." << Utils::ConsoleColors::reset << '\n';
        return false;
    }
    outFile << content;
    return true;
}