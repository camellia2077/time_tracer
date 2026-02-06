// infrastructure/io/core/file_writer.cpp
#include "infrastructure/io/core/file_writer.hpp"

#include <fstream>
#include <stdexcept>

void FileWriter::WriteContent(const std::filesystem::path& path,
                               const std::string& content) {

  std::ofstream file(path, std::ios::out | std::ios::trunc);

  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for writing: " +
                             path.string());
  }

  file << content;

  if (file.fail()) {
    throw std::runtime_error("Error occurred while writing to file: " +
                             path.string());
  }
}
