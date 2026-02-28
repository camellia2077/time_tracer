// infrastructure/crypto/internal/file_crypto_io.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_IO_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_IO_HPP_

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

#include "infrastructure/crypto/file_crypto_service.hpp"
#include "infrastructure/crypto/internal/file_crypto_progress_control.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace fs = std::filesystem;

auto ReadAllBytes(const fs::path& path, ProgressReporter* reporter)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

auto WriteAllBytes(const fs::path& path, const std::vector<std::uint8_t>& bytes)
    -> FileCryptoResult;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_IO_HPP_
