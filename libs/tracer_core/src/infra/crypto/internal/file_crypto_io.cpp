// infra/crypto/internal/file_crypto_io.cpp
#include "infra/crypto/internal/file_crypto_io.hpp"

#include <algorithm>
#include <fstream>

#include "infra/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {
namespace {

constexpr std::size_t kIoChunkSize =
    static_cast<std::size_t>(64U) * static_cast<std::size_t>(1024U);

auto EnsureParentDirectory(const fs::path& path) -> FileCryptoResult {
  const auto kParentPath = path.parent_path();
  if (kParentPath.empty()) {
    return {};
  }
  std::error_code error;
  fs::create_directories(kParentPath, error);
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

  const std::streamsize kSize = input.tellg();
  if (kSize < 0) {
    return {MakeError(FileCryptoError::kInputReadFailed,
                      "Failed to read input file size."),
            {}};
  }
  input.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(kSize));
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const std::size_t kChunk = std::min(kIoChunkSize, bytes.size() - offset);
    if (!input.read(reinterpret_cast<char*>(bytes.data() + offset),
                    static_cast<std::streamsize>(kChunk))) {
      return {MakeError(FileCryptoError::kInputReadFailed,
                        "Failed to read input file bytes."),
              {}};
    }
    offset += kChunk;
    if (reporter != nullptr) {
      if (const auto kProgressResult =
              reporter->UpdateCurrentFileProgress(offset);
          !kProgressResult.ok()) {
        return {kProgressResult, {}};
      }
    }
  }

  if (bytes.empty() && reporter != nullptr) {
    if (const auto kProgressResult = reporter->UpdateCurrentFileProgress(0);
        !kProgressResult.ok()) {
      return {kProgressResult, {}};
    }
  }
  return {{}, std::move(bytes)};
}

auto WriteAllBytes(const fs::path& path, const std::vector<std::uint8_t>& bytes)
    -> FileCryptoResult {
  if (const auto kDirResult = EnsureParentDirectory(path); !kDirResult.ok()) {
    return kDirResult;
  }

  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    return MakeError(FileCryptoError::kOutputWriteFailed,
                     "Failed to open output file.");
  }

  std::size_t offset = 0;
  while (offset < bytes.size()) {
    const std::size_t kChunk = std::min(kIoChunkSize, bytes.size() - offset);
    output.write(reinterpret_cast<const char*>(bytes.data() + offset),
                 static_cast<std::streamsize>(kChunk));
    if (!output.good()) {
      return MakeError(FileCryptoError::kOutputWriteFailed,
                       "Failed to write output file.");
    }
    offset += kChunk;
  }
  return {};
}

auto WriteAllBytes(const FileCryptoWriteCallback& write_callback,
                   std::span<const std::uint8_t> bytes) -> FileCryptoResult {
  if (!write_callback) {
    return MakeError(FileCryptoError::kInvalidArgument,
                     "Output writer callback is required.");
  }
  return write_callback(bytes);
}

}  // namespace tracer_core::infrastructure::crypto::internal
