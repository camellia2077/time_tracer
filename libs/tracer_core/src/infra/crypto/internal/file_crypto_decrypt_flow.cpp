// infra/crypto/internal/file_crypto_decrypt_flow.cpp
#include <cstddef>
#include <cstdint>
#include <span>
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

auto DecryptBytesInternal(std::span<const std::uint8_t> encrypted_bytes,
                          std::string_view passphrase,
                          ProgressReporter* reporter,
                          BatchCryptoSession* batch_session)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
  const std::vector<std::uint8_t> kEncryptedBuffer(encrypted_bytes.begin(),
                                                   encrypted_bytes.end());
  TracerFileHeader header{};
  if (const auto kParseResult = ParseHeader(kEncryptedBuffer, header);
      !kParseResult.ok()) {
    return {kParseResult, {}};
  }

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM && defined(TT_HAS_ZSTD) && \
    TT_HAS_ZSTD
  if (const auto kInitResult = InitializeCryptoBackend(); !kInitResult.ok()) {
    return {kInitResult, {}};
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kDeriveKey, true);
        !kPhaseResult.ok()) {
      return {kPhaseResult, {}};
    }
  }

  std::vector<std::uint8_t> key;
  if (header.kdf_id == kKdfArgon2idBatchSubkey) {
    auto [master_result, master_key] =
        GetOrDeriveMasterKeyForHeader(passphrase, header, batch_session);
    if (!master_result.ok()) {
      return {master_result, {}};
    }
    auto [subkey_result, subkey] =
        DeriveSubkeyFromBatchMaster(master_key, header.nonce);
    sodium_memzero(master_key.data(), master_key.size());
    if (!subkey_result.ok()) {
      return {subkey_result, {}};
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
      return {derive_result, {}};
    }
    key = std::move(derived_key);
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kDecrypt, true);
        !kPhaseResult.ok()) {
      sodium_memzero(key.data(), key.size());
      return {kPhaseResult, {}};
    }
  }

  const auto kPayloadBegin =
      kEncryptedBuffer.begin() +
      static_cast<std::ptrdiff_t>(header.header_size);
  const std::vector<std::uint8_t> kCiphertext(kPayloadBegin,
                                              kEncryptedBuffer.end());
  if (kCiphertext.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
    sodium_memzero(key.data(), key.size());
    return {
        MakeError(FileCryptoError::kUnsupportedFormat,
                  "Encrypted payload is too small."),
        {}};
  }

  std::vector<std::uint8_t> decrypted_payload(kCiphertext.size());
  std::uint64_t decrypted_size = 0;
  if (crypto_aead_xchacha20poly1305_ietf_decrypt(
          decrypted_payload.data(), &decrypted_size, nullptr,
          kCiphertext.data(),
          static_cast<std::uint64_t>(kCiphertext.size()), nullptr, 0,
          header.nonce.data(), key.data()) != 0) {
    sodium_memzero(key.data(), key.size());
    return {MakeError(
                FileCryptoError::kDecryptFailed,
                "Decryption failed (wrong passphrase or corrupted ciphertext)."),
            {}};
  }
  sodium_memzero(key.data(), key.size());
  decrypted_payload.resize(static_cast<std::size_t>(decrypted_size));

  std::vector<std::uint8_t> plaintext;
  if (header.kVersion == kFormatVersionV1 ||
      header.compression_id == kCompressionNone) {
    plaintext = std::move(decrypted_payload);
  } else if (header.compression_id == kCompressionZstd) {
    if (reporter != nullptr) {
          if (const auto kPhaseResult =
              reporter->SetPhase(FileCryptoPhase::kDecompress, true);
          !kPhaseResult.ok()) {
        return {kPhaseResult, {}};
      }
    }
    auto [decompress_result, decompressed] =
        DecompressWithZstd(decrypted_payload, header.plaintext_size);
    if (!decompress_result.ok()) {
      return {decompress_result, {}};
    }
    plaintext = std::move(decompressed);
  } else {
    return {MakeError(FileCryptoError::kUnsupportedFormat,
                      "Unsupported compression identifier."),
            {}};
  }
  return {{}, std::move(plaintext)};
#else
  (void)passphrase;
  (void)reporter;
  (void)batch_session;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity,bugprone-easily-swappable-parameters)
auto DecryptFileInternal(const fs::path& input_tracer_path,
                         const fs::path& output_txt_path,
                         std::string_view passphrase,
                         ProgressReporter* reporter,
                         BatchCryptoSession* batch_session)
    -> FileCryptoResult {
  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kReadInput, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }

  auto [read_result, encrypted_bytes] =
      ReadAllBytes(input_tracer_path, reporter);
  if (!read_result.ok()) {
    return read_result;
  }

  auto [decrypt_result, plaintext] =
      DecryptBytesInternal(encrypted_bytes, passphrase, reporter,
                           batch_session);
  if (!decrypt_result.ok()) {
    return decrypt_result;
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kWriteOutput, true);
        !kPhaseResult.ok()) {
      return kPhaseResult;
    }
  }
  const auto kWriteResult = WriteAllBytes(output_txt_path, plaintext);
  if (!kWriteResult.ok()) {
    return kWriteResult;
  }

  if (reporter != nullptr) {
    return reporter->FinishCurrentFile();
  }
  return {};
}

}  // namespace tracer_core::infrastructure::crypto::internal
