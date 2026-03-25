#ifndef TRACER_CORE_INFRA_INTERNAL_CANONICAL_FILE_IO_HPP_
#define TRACER_CORE_INFRA_INTERNAL_CANONICAL_FILE_IO_HPP_

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "shared/utils/canonical_text.hpp"

namespace tracer::core::infrastructure::internal::file_io {

inline auto ReadBytes(const std::filesystem::path& path)
    -> std::vector<std::uint8_t> {
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("File not found: " + path.string());
  }

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for reading: " +
                             path.string());
  }

  const std::vector<std::uint8_t> bytes = {std::istreambuf_iterator<char>(file),
                                           std::istreambuf_iterator<char>()};
  if (file.bad()) {
    throw std::runtime_error("Error occurred while reading file: " +
                             path.string());
  }

  return bytes;
}

inline auto ReadCanonicalText(const std::filesystem::path& path)
    -> std::string {
  return tracer::core::shared::canonical_text::RequireCanonicalText(
      ReadBytes(path), path.string());
}

inline auto WriteBytes(const std::filesystem::path& path,
                       std::span<const std::uint8_t> bytes) -> void {
  std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for writing: " +
                             path.string());
  }

  if (!bytes.empty()) {
    file.write(reinterpret_cast<const char*>(bytes.data()),  // NOLINT
               static_cast<std::streamsize>(bytes.size()));
  }

  if (file.fail()) {
    throw std::runtime_error("Error occurred while writing to file: " +
                             path.string());
  }
}

inline auto WriteCanonicalText(const std::filesystem::path& path,
                               std::string_view content) -> void {
  const std::string canonical =
      tracer::core::shared::canonical_text::RequireCanonicalText(content,
                                                                 path.string());
  const auto bytes =
      tracer::core::shared::canonical_text::ToUtf8Bytes(canonical);
  WriteBytes(path, bytes);
}

}  // namespace tracer::core::infrastructure::internal::file_io

#endif  // TRACER_CORE_INFRA_INTERNAL_CANONICAL_FILE_IO_HPP_
