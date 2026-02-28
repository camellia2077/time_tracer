// infrastructure/crypto/internal/file_crypto_format_compat.hpp
#ifndef INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_FORMAT_COMPAT_HPP_
#define INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_FORMAT_COMPAT_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "infrastructure/crypto/file_crypto_service.hpp"

namespace tracer_core::infrastructure::crypto::internal {

constexpr std::array<char, 4> kTracerMagic = {'T', 'T', 'R', 'C'};
constexpr std::uint8_t kFormatVersionV1 = 1;
constexpr std::uint8_t kFormatVersionV2 = 2;
constexpr std::uint8_t kKdfArgon2id = 1;
constexpr std::uint8_t kKdfArgon2idBatchSubkey = 2;
constexpr std::uint8_t kCipherXChaCha20Poly1305 = 1;
constexpr std::uint8_t kCompressionNone = 0;
constexpr std::uint8_t kCompressionZstd = 1;
constexpr std::uint8_t kDefaultCompressionLevel = 1;

constexpr std::size_t kHeaderSizeV1 = 64;
constexpr std::size_t kHeaderSizeV2 = 80;
constexpr std::size_t kSaltSize = 16;
constexpr std::size_t kNonceSize = 24;

struct TracerFileHeader {
  std::array<char, 4> magic = kTracerMagic;
  std::uint8_t version = kFormatVersionV2;
  std::uint8_t kdf_id = kKdfArgon2id;
  std::uint8_t cipher_id = kCipherXChaCha20Poly1305;
  std::uint8_t compression_id = kCompressionZstd;
  std::uint8_t compression_level = kDefaultCompressionLevel;
  std::uint32_t ops_limit = 0;
  std::uint32_t mem_limit_kib = 0;
  std::array<std::uint8_t, kSaltSize> salt{};
  std::array<std::uint8_t, kNonceSize> nonce{};
  std::uint64_t plaintext_size = 0;
  std::uint64_t ciphertext_size = 0;
  std::size_t header_size = kHeaderSizeV2;
};

auto BuildHeaderBytes(const TracerFileHeader& header)
    -> std::vector<std::uint8_t>;

auto ParseHeader(const std::vector<std::uint8_t>& data,
                 TracerFileHeader& header) -> FileCryptoResult;

}  // namespace tracer_core::infrastructure::crypto::internal

#endif  // INFRASTRUCTURE_CRYPTO_INTERNAL_FILE_CRYPTO_FORMAT_COMPAT_HPP_
