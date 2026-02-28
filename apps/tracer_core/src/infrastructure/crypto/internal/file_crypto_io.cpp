// infrastructure/crypto/internal/file_crypto_io.cpp
#include "infrastructure/crypto/internal/file_crypto_io.hpp"

#include <algorithm>
#include <fstream>

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {
namespace {

constexpr std::size_t kIoChunkSize = 64U * 1024U;

auto EnsureParentDirectory(const fs::path& path) -> FileCryptoResult {
  const auto parent_path = path.parent_path();
  if (parent_path.empty()) {
    return {};
  }
  std::error_code error;
  fs::create_directories(parent_path, error);
  if (error) {
    return MakeError(FileCryptoError::kOutputWriteFailed,
                     "Failed to create output directory.");
  }
  return {};
}

}  // namespace

auto ReadAllBytes(const fs::path& path, ProgressReporter* reporter)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
  std::ifstream input(path, std::ios::binary | std::ios::ate);
  if (!input.is_open()) {
    return {MakeError(FileCryptoError::kInputReadFailed,
                      "Failed to open input file."),
            {}};
  }

  const std::streamsize size = input.tellg();
  if (size < 0) {
    return {MakeError(FileCryptoError::kInputReadFailed,
                      "Failed to read input file size."),
            {}};
  }
  input.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const std::size_t chunk = std::min(kIoChunkSize, bytes.size() - offset);
    if (!input.read(reinterpret_cast<char*>(bytes.data() + offset),
                    static_cast<std::streamsize>(chunk))) {
      return {MakeError(FileCryptoError::kInputReadFailed,
                        "Failed to read input file bytes."),
              {}};
    }
    offset += chunk;
    if (reporter != nullptr) {
      if (const auto progress_result =
              reporter->UpdateCurrentFileProgress(offset);
          !progress_result.ok()) {
        return {progress_result, {}};
      }
    }
  }

  if (bytes.empty() && reporter != nullptr) {
    if (const auto progress_result = reporter->UpdateCurrentFileProgress(0);
        !progress_result.ok()) {
      return {progress_result, {}};
    }
  }
  return {{}, std::move(bytes)};
}

auto WriteAllBytes(const fs::path& path, const std::vector<std::uint8_t>& bytes)
    -> FileCryptoResult {
  if (const auto dir_result = EnsureParentDirectory(path); !dir_result.ok()) {
    return dir_result;
  }

  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    return MakeError(FileCryptoError::kOutputWriteFailed,
                     "Failed to open output file.");
  }

  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const std::size_t chunk = std::min(kIoChunkSize, bytes.size() - offset);
    output.write(reinterpret_cast<const char*>(bytes.data() + offset),
                 static_cast<std::streamsize>(chunk));
    if (!output.good()) {
      return MakeError(FileCryptoError::kOutputWriteFailed,
                       "Failed to write output file.");
    }
    offset += chunk;
  }
  return {};
}

}  // namespace tracer_core::infrastructure::crypto::internal
