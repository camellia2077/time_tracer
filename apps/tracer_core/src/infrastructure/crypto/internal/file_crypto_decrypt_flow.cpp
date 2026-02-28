// infrastructure/crypto/internal/file_crypto_decrypt_flow.cpp
#include <cstddef>
#include <cstdint>
#include <vector>

#include "infrastructure/crypto/internal/file_crypto_backend_engine.hpp"

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

#include "infrastructure/crypto/internal/file_crypto_backend_engine_internal.hpp"
#include "infrastructure/crypto/internal/file_crypto_common.hpp"
#include "infrastructure/crypto/internal/file_crypto_format_compat.hpp"
#include "infrastructure/crypto/internal/file_crypto_io.hpp"

namespace tracer_core::infrastructure::crypto::internal {

auto DecryptFileInternal(const fs::path& input_tracer_path,
                         const fs::path& output_txt_path,
                         std::string_view passphrase,
                         ProgressReporter* reporter,
                         BatchCryptoSession* batch_session)
    -> FileCryptoResult {
  if (reporter != nullptr) {
    if (const auto phase_result =
            reporter->SetPhase(FileCryptoPhase::kReadInput, true);
        !phase_result.ok()) {
      return phase_result;
    }
  }

  auto [read_result, encrypted_bytes] =
      ReadAllBytes(input_tracer_path, reporter);
  if (!read_result.ok()) {
    return read_result;
  }

  TracerFileHeader header{};
  if (const auto parse_result = ParseHeader(encrypted_bytes, header);
      !parse_result.ok()) {
    return parse_result;
  }

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM && defined(TT_HAS_ZSTD) && \
    TT_HAS_ZSTD
  if (const auto init_result = InitializeCryptoBackend(); !init_result.ok()) {
    return init_result;
  }

  if (reporter != nullptr) {
    if (const auto phase_result =
            reporter->SetPhase(FileCryptoPhase::kDeriveKey, true);
        !phase_result.ok()) {
      return phase_result;
    }
  }

  std::vector<std::uint8_t> key;
  if (header.kdf_id == kKdfArgon2idBatchSubkey) {
    auto [master_result, master_key] =
        GetOrDeriveMasterKeyForHeader(passphrase, header, batch_session);
    if (!master_result.ok()) {
      return master_result;
    }
    auto [subkey_result, subkey] =
        DeriveSubkeyFromBatchMaster(master_key, header.nonce);
    sodium_memzero(master_key.data(), master_key.size());
    if (!subkey_result.ok()) {
      return subkey_result;
    }
    key = std::move(subkey);
  } else {
    auto [derive_result, derived_key] = DeriveMasterKeyWithArgon2id(
        passphrase, header.ops_limit, header.mem_limit_kib, header.salt);
    if (!derive_result.ok()) {
      return derive_result;
    }
    key = std::move(derived_key);
  }

  if (reporter != nullptr) {
    if (const auto kPhaseResult =
            reporter->SetPhase(FileCryptoPhase::kDecrypt, true);
        !kPhaseResult.ok()) {
      sodium_memzero(key.data(), key.size());
      return kPhaseResult;
    }
  }

  const auto kPayloadBegin =
      encrypted_bytes.begin() + static_cast<std::ptrdiff_t>(header.header_size);
  const std::vector<std::uint8_t> kCiphertext(kPayloadBegin,
                                              encrypted_bytes.end());
  if (kCiphertext.size() < crypto_aead_xchacha20poly1305_ietf_ABYTES) {
    sodium_memzero(key.data(), key.size());
    return MakeError(FileCryptoError::kUnsupportedFormat,
                     "Encrypted payload is too small.");
  }

  std::vector<std::uint8_t> decrypted_payload(kCiphertext.size());
  unsigned long long decrypted_size = 0;
  if (crypto_aead_xchacha20poly1305_ietf_decrypt(
          decrypted_payload.data(), &decrypted_size, nullptr,
          kCiphertext.data(),
          static_cast<unsigned long long>(kCiphertext.size()), nullptr, 0,
          header.nonce.data(), key.data()) != 0) {
    sodium_memzero(key.data(), key.size());
    return MakeError(
        FileCryptoError::kDecryptFailed,
        "Decryption failed (wrong passphrase or corrupted ciphertext).");
  }
  sodium_memzero(key.data(), key.size());
  decrypted_payload.resize(static_cast<std::size_t>(decrypted_size));

  std::vector<std::uint8_t> plaintext;
  if (header.version == kFormatVersionV1 ||
      header.compression_id == kCompressionNone) {
    plaintext = std::move(decrypted_payload);
  } else if (header.compression_id == kCompressionZstd) {
    if (reporter != nullptr) {
      if (const auto kPhaseResult =
              reporter->SetPhase(FileCryptoPhase::kDecompress, true);
          !kPhaseResult.ok()) {
        return kPhaseResult;
      }
    }
    auto [decompress_result, decompressed] =
        DecompressWithZstd(decrypted_payload, header.plaintext_size);
    if (!decompress_result.ok()) {
      return decompress_result;
    }
    plaintext = std::move(decompressed);
  } else {
    return MakeError(FileCryptoError::kUnsupportedFormat,
                     "Unsupported compression identifier.");
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
#else
  (void)output_txt_path;
  (void)passphrase;
  (void)reporter;
  (void)batch_session;
  return MakeError(
      FileCryptoError::kCryptoBackendUnavailable,
      "File crypto backend is unavailable (build without libsodium/zstd).");
#endif
}

}  // namespace tracer_core::infrastructure::crypto::internal
