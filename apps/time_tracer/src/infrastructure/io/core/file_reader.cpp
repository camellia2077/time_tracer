// infrastructure/io/core/file_reader.cpp

#include "infrastructure/io/core/file_reader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

auto FileReader::ReadContent(const std::filesystem::path& path)
    -> std::string {
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("File not found: " + path.string());
  }

  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for reading: " +
                             path.string());
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  if (file.bad()) {
    throw std::runtime_error("Error occurred while reading file: " +
                             path.string());
  }

  return buffer.str();
}
