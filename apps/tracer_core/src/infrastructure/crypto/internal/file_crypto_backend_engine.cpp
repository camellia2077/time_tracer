// infrastructure/crypto/internal/file_crypto_backend_engine.cpp
#include "infrastructure/crypto/internal/file_crypto_backend_engine.hpp"

#include <cstdint>
#include <limits>
#include <utility>

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

#include "infrastructure/crypto/internal/file_crypto_backend_engine_internal.hpp"
#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {

auto BuildEncryptBatchCryptoSession(std::string_view passphrase,
                                    FileCryptoSecurityLevel security_level)
    -> std::pair<FileCryptoResult, BatchCryptoSession> {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  if (passphrase.empty()) {
    return {MakeError(FileCryptoError::kInvalidArgument,
                      "Passphrase must not be empty."),
            {}};
  }
  if (const auto kInitResult = InitializeCryptoBackend(); !kInitResult.ok()) {
    return {kInitResult, {}};
  }

  const PwhashLimitPair kLimits = ResolvePwhashLimits(security_level);
  if (kLimits.ops_limit == 0 || kLimits.mem_limit_bytes == 0) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Invalid KDF limits for batch session."),
            {}};
  }
  if (kLimits.ops_limit > static_cast<unsigned long long>(
                              std::numeric_limits<std::uint32_t>::max())) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Batch session opslimit exceeds header capacity."),
            {}};
  }

  const auto kMemLimitKib = kLimits.mem_limit_bytes / 1024ULL;
  if (kMemLimitKib == 0 ||
      kMemLimitKib > static_cast<unsigned long long>(
                         std::numeric_limits<std::uint32_t>::max())) {
    return {MakeError(FileCryptoError::kCryptoOperationFailed,
                      "Batch session memlimit exceeds header capacity."),
            {}};
  }

  BatchCryptoSession session{};
  session.encrypt_mode_enabled = true;
  randombytes_buf(session.encrypt_batch_salt.data(),
                  session.encrypt_batch_salt.size());

  auto [derive_result, master_key] = DeriveMasterKeyWithArgon2id(
      passphrase, static_cast<std::uint32_t>(kLimits.ops_limit),
      static_cast<std::uint32_t>(kMemLimitKib), session.encrypt_batch_salt);
  if (!derive_result.ok()) {
    return {derive_result, {}};
  }
  session.encrypt_master_key = std::move(master_key);
  return {{}, std::move(session)};
#else
  (void)passphrase;
  (void)security_level;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

}  // namespace tracer_core::infrastructure::crypto::internal
