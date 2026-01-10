// io/core/FileWriter.cpp
#include "FileWriter.hpp"
#include <fstream>
#include <stdexcept>

void FileWriter::write_content(const std::filesystem::path& path, const std::string& content) {
    // 使用 std::ios::trunc 确保覆盖旧内容
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file for writing: " + path.string());
    }

    file << content;

    if (file.fail()) {
        throw std::runtime_error("Error occurred while writing to file: " + path.string());
    }
}