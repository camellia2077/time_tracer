// io/FileManager.cpp
#ifndef IO_FILEMANAGER_H
#define IO_FILEMANAGER_H

#include "io/FileManager.h"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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
        std::cerr << RED_COLOR << "Error creating directories. Detail: " << e.what() << RESET_COLOR << '\n';
        return false;
    }
    return true;
}

bool FileManager::write_log_file(const std::filesystem::path& file_path, const std::string& content) {
    std::ofstream outFile(file_path);
    if (!outFile.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open file '" << file_path.string() << "' for writing." << RESET_COLOR << '\n';
        return false;
    }
    outFile << content;
    return true;
}

// [新增] 实现读取
std::optional<std::string> FileManager::read_file(const std::filesystem::path& file_path) {
    std::ifstream inFile(file_path);
    if (!inFile.is_open()) {
        // 这里的错误打印可以保留，或者留给调用者处理。
        // 为了统一体验，这里打印IO层面的错误。
        std::cerr << RED_COLOR << "Error: Could not open file '" << file_path.string() << "' for reading." << RESET_COLOR << '\n';
        return std::nullopt;
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

#endif // IO_FILEMANAGER_H