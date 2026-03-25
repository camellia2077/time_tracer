// infra/crypto/internal/file_crypto_encrypt_flow.cpp
#include <cstddef>
#include <cstdint>
#include <vector>

#include "infra/crypto/internal/file_crypto_backend_engine.hpp"

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

#include "infra/crypto/internal/file_crypto_backend_engine_internal.hpp"
#include "infra/crypto/internal/file_crypto_common.hpp"
#include "infra/crypto/internal/file_crypto_format_compat.hpp"
#include "infra/crypto/internal/file_crypto_io.hpp"

namespace tracer_core::infrastructure::crypto::internal {

// NOLINTNEXTLINE(readability-function-cognitive-complexity,bugprone-easily-swappable-parameters)
auto EncryptFileInternal(const fs::path& input_txt_path,
                         const fs::path& output_tracer_path,
                         std::string_view passphrase,
                         FileCryptoSecurityLevel security_level,
                         ProgressReporter* reporter,
                         BatchCryptoSession* batch_session)
    -> FileCryptoResult {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM && defined(TT_HAS_ZSTD) && \
    TT_HAS_ZSTD
  if (const auto kInitResult = InitializeCryptoBackend(); !kInitResult.ok()) {
    return kInitResult;
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kReadInput, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }
  auto [read_result, plaintext] = ReadAllBytes(input_txt_path, reporter);
  if (!read_result.ok()) {
    return read_result;
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kCompress, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }
  auto [compress_result, compressed_plaintext] =
      CompressWithZstdLevel1(plaintext);
  if (!compress_result.ok()) {
    return compress_result;
  }

  const bool kUseBatchSubkey = batch_session != nullptr &&
                               batch_session->encrypt_mode_enabled &&
                               batch_session->encrypt_master_key.size() ==
                                   crypto_aead_xchacha20poly1305_ietf_KEYBYTES;
  auto [header_result, header] = BuildDefaultHeaderV2(
      security_level, kUseBatchSubkey ? kKdfArgon2idBatchSubkey : kKdfArgon2id,
      kUseBatchSubkey ? &batch_session->encrypt_batch_salt : nullptr);
  if (!header_result.ok()) {
    return header_result;
  }
  header.plaintext_size = static_cast<std::uint64_t>(plaintext.size());

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kDeriveKey, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }

  std::vector<std::uint8_t> key;
  if (kUseBatchSubkey) {
    auto [subkey_result, subkey] = DeriveSubkeyFromBatchMaster(
        batch_session->encrypt_master_key, header.nonce);
    if (!subkey_result.ok()) {
      return subkey_result;
    }
    key = std::move(subkey);
  } else {
    auto [derive_result, derived_key] =
        DeriveMasterKeyWithArgon2id(passphrase,
                                    Argon2idLimits{
                                        .ops_limit = header.ops_limit,
                                        .mem_limit_kib = header.mem_limit_kib,
                                    },
                                    header.salt);
    if (!derive_result.ok()) {
      return derive_result;
    }
    key = std::move(derived_key);
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kEncrypt, true);
        !kPhaseResult.ok()) {
      sodium_memzero(key.data(), key.size());
      return kPhaseResult;
    }
  }

  std::vector<std::uint8_t> ciphertext(
      compressed_plaintext.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES);
  unsigned long long ciphertext_size = 0;
  if (crypto_aead_xchacha20poly1305_ietf_encrypt(
          ciphertext.data(), &ciphertext_size, compressed_plaintext.data(),
          static_cast<unsigned long long>(compressed_plaintext.size()), nullptr,
          0, nullptr, header.nonce.data(), key.data()) != 0) {
    sodium_memzero(key.data(), key.size());
    return MakeError(FileCryptoError::kCryptoOperationFailed,
                     "Encryption failed.");
  }
  sodium_memzero(key.data(), key.size());
  ciphertext.resize(static_cast<std::size_t>(ciphertext_size));
  header.ciphertext_size = static_cast<std::uint64_t>(ciphertext.size());

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kWriteOutput, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }
  auto output_bytes = BuildHeaderBytes(header);
  output_bytes.insert(output_bytes.end(), ciphertext.begin(), ciphertext.end());
  const auto kWriteResult = WriteAllBytes(output_tracer_path, output_bytes);
  if (!kWriteResult.ok()) {
    return kWriteResult;
  }

  if (reporter != nullptr) {
    return reporter->FinishCurrentFile();
  }
  return {};
#else
  (void)input_txt_path;
  (void)output_tracer_path;
  (void)passphrase;
  (void)security_level;
  (void)reporter;
  (void)batch_session;
  return MakeError(
      FileCryptoError::kCryptoBackendUnavailable,
      "File crypto backend is unavailable (build without libsodium/zstd).");
#endif
}

}  // namespace tracer_core::infrastructure::crypto::internal
