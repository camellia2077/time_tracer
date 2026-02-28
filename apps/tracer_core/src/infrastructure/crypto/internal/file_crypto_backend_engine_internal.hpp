// infrastructure/crypto/internal/file_crypto_backend_engine_internal.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_INTERNAL_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_INTERNAL_HPP_

#include <array>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/crypto/internal/file_crypto_backend_engine.hpp"
#include "infrastructure/crypto/internal/file_crypto_format_compat.hpp"

namespace tracer_core::infrastructure::crypto::internal {

struct PwhashLimitPair {
  unsigned long long ops_limit = 0;
  unsigned long long mem_limit_bytes = 0;
};

auto InitializeCryptoBackend() -> FileCryptoResult;

auto ResolvePwhashLimits(FileCryptoSecurityLevel security_level)
    -> PwhashLimitPair;

auto DeriveMasterKeyWithArgon2id(
    std::string_view passphrase, std::uint32_t ops_limit,
    std::uint32_t mem_limit_kib,
    const std::array<std::uint8_t, kSaltSize>& salt)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

auto DeriveSubkeyFromBatchMaster(
    const std::vector<std::uint8_t>& master_key,
    const std::array<std::uint8_t, kNonceSize>& nonce)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

auto GetOrDeriveMasterKeyForHeader(std::string_view passphrase,
                                   const TracerFileHeader& header,
                                   BatchCryptoSession* batch_session)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

auto BuildDefaultHeaderV2(
    FileCryptoSecurityLevel security_level, std::uint8_t kdf_id,
    const std::array<std::uint8_t, kSaltSize>* fixed_salt = nullptr)
    -> std::pair<FileCryptoResult, TracerFileHeader>;

auto CompressWithZstdLevel1(const std::vector<std::uint8_t>& plaintext)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

auto DecompressWithZstd(const std::vector<std::uint8_t>& compressed,
                        std::uint64_t expected_plaintext_size)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>>;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_BACKEND_ENGINE_INTERNAL_HPP_
