// infrastructure/crypto/internal/file_crypto_compress.cpp
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "infrastructure/crypto/internal/file_crypto_backend_engine_internal.hpp"

#if defined(TT_HAS_ZSTD) && TT_HAS_ZSTD
#include <zstd.h>
#endif

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {

auto CompressWithZstdLevel1(const std::vector<std::uint8_t>& plaintext)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
#if defined(TT_HAS_ZSTD) && TT_HAS_ZSTD
  const std::size_t bound = ZSTD_compressBound(plaintext.size());
  if (bound == 0) {
    return {MakeError(FileCryptoError::kCompressionFailed,
                      "zstd failed to estimate compression bound."),
            {}};
  }

  std::vector<std::uint8_t> compressed(bound);
  const std::size_t output_size =
      ZSTD_compress(compressed.data(), compressed.size(), plaintext.data(),
                    plaintext.size(), kDefaultCompressionLevel);
  if (ZSTD_isError(output_size)) {
    return {MakeError(FileCryptoError::kCompressionFailed,
                      "zstd compression failed."),
            {}};
  }

  compressed.resize(output_size);
  return {{}, std::move(compressed)};
#else
  (void)plaintext;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

auto DecompressWithZstd(const std::vector<std::uint8_t>& compressed,
                        std::uint64_t expected_plaintext_size)
    -> std::pair<FileCryptoResult, std::vector<std::uint8_t>> {
#if defined(TT_HAS_ZSTD) && TT_HAS_ZSTD
  if (expected_plaintext_size >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    return {MakeError(FileCryptoError::kUnsupportedFormat,
                      "plaintext_size exceeds platform size_t capacity."),
            {}};
  }

  const std::size_t kDstSize =
      static_cast<std::size_t>(expected_plaintext_size);
  std::vector<std::uint8_t> plaintext(kDstSize);
  const std::size_t kDecompressedSize = ZSTD_decompress(
      plaintext.data(), plaintext.size(), compressed.data(), compressed.size());
  if (ZSTD_isError(kDecompressedSize) != 0U) {
    return {MakeError(FileCryptoError::kDecompressionFailed,
                      "zstd decompression failed."),
            {}};
  }
  if (kDecompressedSize != kDstSize) {
    return {MakeError(FileCryptoError::kCompressionMetadataMismatch,
                      "Decompressed size does not match plaintext_size."),
            {}};
  }

  return {{}, std::move(plaintext)};
#else
  (void)compressed;
  (void)expected_plaintext_size;
  return {MakeError(FileCryptoError::kCryptoBackendUnavailable,
                    "File crypto backend is unavailable (build without "
                    "libsodium/zstd)."),
          {}};
#endif
}

}  // namespace tracer_core::infrastructure::crypto::internal
