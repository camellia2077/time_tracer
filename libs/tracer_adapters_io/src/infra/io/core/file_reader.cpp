// infra/io/core/file_reader.cpp

#include "infra/io/core/file_reader.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>

#include "shared/utils/canonical_text.hpp"

auto FileReader::ReadBytes(const std::filesystem::path& path)
    -> std::vector<std::uint8_t> {
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("File not found: " + path.string());
  }

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for reading: " +
                             path.string());
  }

  const std::vector<std::uint8_t> bytes = {
      std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  if (file.bad()) {
    throw std::runtime_error("Error occurred while reading file: " +
                             path.string());
  }

  return bytes;
}

auto FileReader::ReadCanonicalText(const std::filesystem::path& path)
    -> std::string {
  return tracer::core::shared::canonical_text::RequireCanonicalText(
      ReadBytes(path), path.string());
}
