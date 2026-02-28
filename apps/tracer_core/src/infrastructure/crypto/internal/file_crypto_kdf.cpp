// infrastructure/crypto/internal/file_crypto_kdf.cpp
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#include "infrastructure/crypto/internal/file_crypto_backend_engine_internal.hpp"

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {
namespace {

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
constexpr std::array<std::uint8_t, 8> kBatchSubkeyLabel = {
    static_cast<std::uint8_t>('T'), static_cast<std::uint8_t>('T'),
    static_cast<std::uint8_t>('B'), static_cast<std::uint8_t>('S'),
    static_cast<std::uint8_t>('U'), static_cast<std::uint8_t>('B'),
    static_cast<std::uint8_t>('K'), static_cast<std::uint8_t>('1'),
};
#endif

auto FindCachedMasterKey(BatchCryptoSession* batch_session,
                         const TracerFileHeader& header)
    -> std::vector<std::uint8_t>* {
  if (batch_session == nullptr) {
    return nullptr;
  }
  for (auto& entry : batch_session->decrypt_master_key_cache) {
    if (entry.kdf_id == header.kdf_id && entry.ops_limit == header.ops_limit &&
        entry.mem_limit_kib == header.mem_limit_kib &&
        entry.salt == header.salt) {
      return &entry.master_key;
    }
  }
  return nullptr;
}

auto CacheMasterKey(BatchCryptoSession* batch_session,
                    const TracerFileHeader& header,
                    const std::vector<std::uint8_t>& master_key) -> void {
  if (batch_session == nullptr) {
    return;
  }
  BatchMasterKeyCacheEntry entry{};
  entry.kdf_id = header.kdf_id;
  entry.ops_limit = header.ops_limit;
  entry.mem_limit_kib = header.mem_limit_kib;
  entry.salt = header.salt;
  entry.master_key = master_key;
  batch_session->decrypt_master_key_cache.push_back(std::move(entry));
}

}  // namespace

auto InitializeCryptoBackend() -> FileCryptoResult {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  if (sodium_init() < 0) {
    return MakeError(FileCryptoError::kCryptoInitializationFailed,
                     "libsodium initialization failed.");
  }
  return {};
#else
  return MakeError(FileCryptoError::kCryptoBackendUnavailable,
                   "File crypto backend is unavailable (build without "
                   "libsodium/zstd).");
#endif
}

auto ResolvePwhashLimits(FileCryptoSecurityLevel security_level)
    -> PwhashLimitPair {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  switch (security_level) {
    case FileCryptoSecurityLevel::kHigh:
      return {
          static_cast<unsigned long long>(crypto_pwhash_OPSLIMIT_SENSITIVE),
          static_cast<unsigned long long>(crypto_pwhash_MEMLIMIT_SENSITIVE),
      };
    case FileCryptoSecurityLevel::kModerate:
      return {
          static_cast<unsigned long long>(crypto_pwhash_OPSLIMIT_MODERATE),
          static_cast<unsigned long long>(crypto_pwhash_MEMLIMIT_MODERATE),
      };
    case FileCryptoSecurityLevel::kInteractive:
    default:
      return {
          static_cast<unsigned long long>(crypto_pwhash_OPSLIMIT_INTERACTIVE),
          static_cast<unsigned long long>(crypto_pwhash_MEMLIMIT_INTERACTIVE),
      };
  }
#else
  (void)security_level;
  return {};
#endif
}

auto DeriveMasterKeyWithArgon2id(
    std::string_view passphrase, std::uint32_t ops_limit,
    std::uint32_t mem_limit_kib,
    const std::array<std::uint8_t, kSaltSize>& salt)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  std::vector<std::uint8_t> key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  const std::size_t mem_limit_bytes =
      static_cast<std::size_t>(mem_limit_kib) * 1024U;
  if (crypto_pwhash(key.data(), key.size(), passphrase.data(),
                    static_cast<unsigned long long>(passphrase.size()),
                    salt.data(), static_cast<unsigned long long>(ops_limit),
                    mem_limit_bytes, crypto_pwhash_ALG_ARGON2ID13) != 0) {
    sodium_memzero(key.data(), key.size());
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Failed to derive key with Argon2id."),
            {}};
  }
  return {{}, std::move(key)};
#else
  (void)passphrase;
  (void)ops_limit;
  (void)mem_limit_kib;
  (void)salt;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

auto DeriveSubkeyFromBatchMaster(
    const std::vector<std::uint8_t>& master_key,
    const std::array<std::uint8_t, kNonceSize>& nonce)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  if (master_key.size() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Batch master key has unexpected size."),
            {}};
  }

  std::array<std::uint8_t, kBatchSubkeyLabel.size() + kNonceSize> input{};
  std::copy(kBatchSubkeyLabel.begin(), kBatchSubkeyLabel.end(), input.begin());
  std::copy(nonce.begin(), nonce.end(),
            input.begin() + kBatchSubkeyLabel.size());

  std::vector<std::uint8_t> subkey(crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
  if (crypto_generichash(subkey.data(), subkey.size(), input.data(),
                         input.size(), master_key.data(),
                         master_key.size()) != 0) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Failed to derive batch subkey."),
            {}};
  }
  return {{}, std::move(subkey)};
#else
  (void)master_key;
  (void)nonce;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

auto GetOrDeriveMasterKeyForHeader(std::string_view passphrase,
                                   const TracerFileHeader& header,
                                   BatchCryptoSession* batch_session)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
  if (auto* cached = FindCachedMasterKey(batch_session, header);
      cached != nullptr) {
    return {{}, *cached};
  }
  auto [derive_result, derived_key] = DeriveMasterKeyWithArgon2id(
      passphrase, header.ops_limit, header.mem_limit_kib, header.salt);
  if (!derive_result.ok()) {
    return {derive_result, {}};
  }
  CacheMasterKey(batch_session, header, derived_key);
  return {{}, std::move(derived_key)};
}

auto BuildDefaultHeaderV2(FileCryptoSecurityLevel security_level,
                          std::uint8_t kdf_id,
                          const std::array<std::uint8_t, kSaltSize>* fixed_salt)
    -> std::pair<FileCryptoResult, TracerFileHeader> {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  TracerFileHeader header{};

  constexpr std::size_t kOpsLimitMax =
      std::numeric_limits<std::uint32_t>::max();
  const PwhashLimitPair limits = ResolvePwhashLimits(security_level);
  const auto kOpsLimit = limits.ops_limit;
  if (kOpsLimit > kOpsLimitMax) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "libsodium opslimit exceeds v2 header capacity."),
            {}};
  }

  const auto kMemLimitBytes = limits.mem_limit_bytes;
  const auto kMemLimitKiB = kMemLimitBytes / 1024ULL;
  if (kMemLimitKiB == 0 ||
      kMemLimitKiB > static_cast<unsigned long long>(
                         std::numeric_limits<std::uint32_t>::max())) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "libsodium memlimit exceeds v2 header capacity."),
            {}};
  }

  header.version = kFormatVersionV2;
  header.kdf_id = kdf_id;
  header.cipher_id = kCipherXChaCha20Poly1305;
  header.compression_id = kCompressionZstd;
  header.compression_level = kDefaultCompressionLevel;
  header.ops_limit = static_cast<std::uint32_t>(kOpsLimit);
  header.mem_limit_kib = static_cast<std::uint32_t>(kMemLimitKiB);
  header.header_size = kHeaderSizeV2;
  if (fixed_salt != nullptr) {
    header.salt = *fixed_salt;
  } else {
    randombytes_buf(header.salt.data(), header.salt.size());
  }
  randombytes_buf(header.nonce.data(), header.nonce.size());
  return {{}, std::move(header)};
#else
  (void)security_level;
  (void)kdf_id;
  (void)fixed_salt;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

}  // namespace tracer_core::infrastructure::crypto::internal
