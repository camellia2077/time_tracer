// infrastructure/crypto/internal/file_crypto_directory_orchestrator.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_ORCHESTRATOR_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_ORCHESTRATOR_HPP_

#include <filesystem>
#include <string_view>

#include "infrastructure/crypto/file_crypto_service.hpp"
#include "infrastructure/crypto/internal/file_crypto_directory_plan.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace fs = std::filesystem;

auto RunDirectoryCrypto(FileCryptoOperation operation,
                        const fs::path& input_root_path,
                        const fs::path& output_root_path,
                        std::string_view passphrase,
                        const DirectoryCryptoExtensions& extensions,
                        const FileCryptoOptions& options)
    -> FileCryptoBatchResult;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_DIRECTORY_ORCHESTRATOR_HPP_
