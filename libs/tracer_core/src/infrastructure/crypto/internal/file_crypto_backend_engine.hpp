// infrastructure/crypto/internal/file_crypto_backend_engine.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_HPP_

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/crypto/file_crypto_service.hpp"
#include "infrastructure/crypto/internal/file_crypto_progress_control.hpp"

namespace tracer_core::infrastructure::crypto::internal {

namespace fs = std::filesystem;

struct BatchMasterKeyCacheEntry {
  std::uint8_t kdf_id = 0;
  std::uint32_t ops_limit = 0;
  std::uint32_t mem_limit_kib = 0;
  std::array<std::uint8_t, 16> salt{};
  std::vector<std::uint8_t> master_key;
};

struct BatchCryptoSession {
  bool encrypt_mode_enabled = false;
  std::array<std::uint8_t, 16> encrypt_batch_salt{};
  std::vector<std::uint8_t> encrypt_master_key;
  std::vector<BatchMasterKeyCacheEntry> decrypt_master_key_cache;
};

auto BuildEncryptBatchCryptoSession(std::string_view passphrase,
                                    FileCryptoSecurityLevel security_level)
    -> std::pair<FileCryptoResult, BatchCryptoSession>;

auto EncryptFileInternal(const fs::path& input_txt_path,
                         const fs::path& output_tracer_path,
                         std::string_view passphrase,
                         FileCryptoSecurityLevel security_level,
                         ProgressReporter* reporter,
                         BatchCryptoSession* batch_session = nullptr)
    -> FileCryptoResult;

auto DecryptFileInternal(const fs::path& input_tracer_path,
                         const fs::path& output_txt_path,
                         std::string_view passphrase,
                         ProgressReporter* reporter,
                         BatchCryptoSession* batch_session = nullptr)
    -> FileCryptoResult;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_HPP_
