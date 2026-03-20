// infrastructure/io/core/file_writer.cpp
#include "infrastructure/io/core/file_writer.hpp"

#include <fstream>
#include <stdexcept>

#include "shared/utils/canonical_text.hpp"

void FileWriter::WriteBytes(const std::filesystem::path& path,
                            std::span<const std::uint8_t> bytes) {
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

void FileWriter::WriteCanonicalText(const std::filesystem::path& path,
                                    std::string_view content) {
  const std::string canonical =
      tracer::core::shared::canonical_text::RequireCanonicalText(content,
                                                                 path.string());
  const auto bytes = tracer::core::shared::canonical_text::ToUtf8Bytes(canonical);
  WriteBytes(path, bytes);
}
