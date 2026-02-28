// infrastructure/crypto/internal/file_crypto_format_compat.cpp
#include "infrastructure/crypto/internal/file_crypto_format_compat.hpp"

#include <algorithm>
#include <array>

#include "infrastructure/crypto/internal/file_crypto_common.hpp"

namespace tracer_core::infrastructure::crypto::internal {
namespace {

constexpr std::size_t kV2PlaintextSizeOffset = 60;
constexpr std::size_t kV2CiphertextSizeOffset = 68;
constexpr std::array<std::size_t, 7> kV2ReservedOffsets = {9,  10, 11, 76,
                                                           77, 78, 79};
constexpr std::uint32_t kByteMask = 0xFFU;
constexpr unsigned kShiftBits8 = 8U;
constexpr unsigned kShiftBits16 = 16U;
constexpr unsigned kShiftBits24 = 24U;
constexpr std::size_t kU64ByteCount = 8;
constexpr unsigned kByteShiftStep = 8U;
constexpr std::size_t kVersionOffset = 4;
constexpr std::size_t kV1KdfOffset = 5;
constexpr std::size_t kV1CipherOffset = 6;
constexpr std::size_t kV1ReservedOffset = 7;
constexpr std::size_t kV1OpsLimitOffset = 8;
constexpr std::size_t kV1MemLimitOffset = 12;
constexpr std::size_t kV1SaltOffset = 16;
constexpr std::size_t kV1NonceOffset = 32;
constexpr std::size_t kV1CiphertextSizeOffset = 56;
constexpr std::size_t kV2KdfOffset = 5;
constexpr std::size_t kV2CipherOffset = 6;
constexpr std::size_t kV2CompressionIdOffset = 7;
constexpr std::size_t kV2CompressionLevelOffset = 8;
constexpr std::size_t kV2OpsLimitOffset = 12;
constexpr std::size_t kV2MemLimitOffset = 16;
constexpr std::size_t kV2SaltOffset = 20;
constexpr std::size_t kV2NonceOffset = 36;

void AppendU32LE(std::vector<std::uint8_t>& out, std::uint32_t value) {
  out.push_back(static_cast<std::uint8_t>(value & kByteMask));
  out.push_back(static_cast<std::uint8_t>((value >> kShiftBits8) & kByteMask));
  out.push_back(static_cast<std::uint8_t>((value >> kShiftBits16) & kByteMask));
  out.push_back(static_cast<std::uint8_t>((value >> kShiftBits24) & kByteMask));
}

void AppendU64LE(std::vector<std::uint8_t>& out, std::uint64_t value) {
  for (std::size_t index = 0; index < kU64ByteCount; ++index) {
    out.push_back(static_cast<std::uint8_t>(
        (value >> (index * kByteShiftStep)) & kByteMask));
  }
}

auto ReadU32LE(const std::vector<std::uint8_t>& data, std::size_t offset)
    -> std::uint32_t {
  std::uint32_t value = 0;
  value |= static_cast<std::uint32_t>(data[offset + 0]);
  value |= static_cast<std::uint32_t>(data[offset + 1]) << kShiftBits8;
  value |= static_cast<std::uint32_t>(data[offset + 2]) << kShiftBits16;
  value |= static_cast<std::uint32_t>(data[offset + 3]) << kShiftBits24;
  return value;
}

auto ReadU64LE(const std::vector<std::uint8_t>& data, std::size_t offset)
    -> std::uint64_t {
  std::uint64_t value = 0;
  for (std::size_t index = 0; index < kU64ByteCount; ++index) {
    value |= static_cast<std::uint64_t>(data[offset + index])
             << (index * kByteShiftStep);
  }
  return value;
}

}  // namespace

auto BuildHeaderBytes(const TracerFileHeader& header)
    -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> bytes;
  if (header.version == kFormatVersionV1) {
    bytes.reserve(kHeaderSizeV1);
    bytes.insert(bytes.end(), header.magic.begin(), header.magic.end());
    bytes.push_back(header.version);
    bytes.push_back(header.kdf_id);
    bytes.push_back(header.cipher_id);
    bytes.push_back(0);
    AppendU32LE(bytes, header.ops_limit);
    AppendU32LE(bytes, header.mem_limit_kib);
    bytes.insert(bytes.end(), header.salt.begin(), header.salt.end());
    bytes.insert(bytes.end(), header.nonce.begin(), header.nonce.end());
    AppendU64LE(bytes, header.ciphertext_size);
    return bytes;
  }

  bytes.reserve(kHeaderSizeV2);
  bytes.insert(bytes.end(), header.magic.begin(), header.magic.end());
  bytes.push_back(header.version);
  bytes.push_back(header.kdf_id);
  bytes.push_back(header.cipher_id);
  bytes.push_back(header.compression_id);
  bytes.push_back(header.compression_level);
  bytes.push_back(0);
  bytes.push_back(0);
  bytes.push_back(0);
  AppendU32LE(bytes, header.ops_limit);
  AppendU32LE(bytes, header.mem_limit_kib);
  bytes.insert(bytes.end(), header.salt.begin(), header.salt.end());
  bytes.insert(bytes.end(), header.nonce.begin(), header.nonce.end());
  AppendU64LE(bytes, header.plaintext_size);
  AppendU64LE(bytes, header.ciphertext_size);
  bytes.push_back(0);
  bytes.push_back(0);
  bytes.push_back(0);
  bytes.push_back(0);
  return bytes;
}

auto ParseHeader(const std::vector<std::uint8_t>& data,
                 TracerFileHeader& header) -> FileCryptoResult {
  if (data.size() < 8) {
    return MakeError(FileCryptoError::kUnsupportedFormat,
                     "Encrypted file is too small to contain a valid header.");
  }

  std::copy_n(data.begin(), header.magic.size(), header.magic.begin());
  if (header.magic != kTracerMagic) {
    return MakeError(FileCryptoError::kUnsupportedFormat,
                     "Invalid encrypted file magic. Expected TTRC.");
  }

  const auto version = data[kVersionOffset];
  if (version == kFormatVersionV1) {
    if (data.size() < kHeaderSizeV1) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted file is too small for v1 header.");
    }
    header.version = version;
    header.kdf_id = data[kV1KdfOffset];
    header.cipher_id = data[kV1CipherOffset];
    const auto reserved = data[kV1ReservedOffset];
    header.ops_limit = ReadU32LE(data, kV1OpsLimitOffset);
    header.mem_limit_kib = ReadU32LE(data, kV1MemLimitOffset);
    std::copy_n(data.begin() + kV1SaltOffset, header.salt.size(),
                header.salt.begin());
    std::copy_n(data.begin() + kV1NonceOffset, header.nonce.size(),
                header.nonce.begin());
    header.plaintext_size = 0;
    header.ciphertext_size = ReadU64LE(data, kV1CiphertextSizeOffset);
    header.compression_id = kCompressionNone;
    header.compression_level = 0;
    header.header_size = kHeaderSizeV1;

    if (header.kdf_id != kKdfArgon2id ||
        header.cipher_id != kCipherXChaCha20Poly1305) {
      return MakeError(
          FileCryptoError::kUnsupportedFormat,
          "Unsupported KDF or cipher identifier in encrypted v1 file.");
    }
    if (reserved != 0) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v1 reserved field must be zero.");
    }
    if (header.ops_limit == 0 || header.mem_limit_kib == 0) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v1 KDF parameters are invalid.");
    }
    const auto kPayloadSize =
        static_cast<std::uint64_t>(data.size() - header.header_size);
    if (header.ciphertext_size != kPayloadSize) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v1 ciphertext size does not match payload.");
    }
    return {};
  }

  if (version == kFormatVersionV2) {
    if (data.size() < kHeaderSizeV2) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted file is too small for v2 header.");
    }
    header.version = version;
    header.kdf_id = data[kV2KdfOffset];
    header.cipher_id = data[kV2CipherOffset];
    header.compression_id = data[kV2CompressionIdOffset];
    header.compression_level = data[kV2CompressionLevelOffset];
    header.ops_limit = ReadU32LE(data, kV2OpsLimitOffset);
    header.mem_limit_kib = ReadU32LE(data, kV2MemLimitOffset);
    std::copy_n(data.begin() + kV2SaltOffset, header.salt.size(),
                header.salt.begin());
    std::copy_n(data.begin() + kV2NonceOffset, header.nonce.size(),
                header.nonce.begin());
    header.plaintext_size = ReadU64LE(data, kV2PlaintextSizeOffset);
    header.ciphertext_size = ReadU64LE(data, kV2CiphertextSizeOffset);
    header.header_size = kHeaderSizeV2;

    if ((header.kdf_id != kKdfArgon2id &&
         header.kdf_id != kKdfArgon2idBatchSubkey) ||
        header.cipher_id != kCipherXChaCha20Poly1305) {
      return MakeError(
          FileCryptoError::kUnsupportedFormat,
          "Unsupported KDF or cipher identifier in encrypted v2 file.");
    }
    if (header.compression_id != kCompressionZstd) {
      return MakeError(
          FileCryptoError::kUnsupportedFormat,
          "Unsupported compression identifier in encrypted v2 file.");
    }
    if (header.compression_level == 0) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v2 compression level must be non-zero.");
    }
    if (!std::all_of(
            kV2ReservedOffsets.begin(), kV2ReservedOffsets.end(),
            [&](std::size_t index) -> bool { return data[index] == 0; })) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v2 reserved fields must be zero.");
    }
    if (header.ops_limit == 0 || header.mem_limit_kib == 0) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v2 KDF parameters are invalid.");
    }
    const auto kPayloadSize =
        static_cast<std::uint64_t>(data.size() - header.header_size);
    if (header.ciphertext_size != kPayloadSize) {
      return MakeError(FileCryptoError::kUnsupportedFormat,
                       "Encrypted v2 ciphertext size does not match payload.");
    }
    return {};
  }

  return MakeError(FileCryptoError::kUnsupportedFormat,
                   "Unsupported encrypted file version.");
}

}  // namespace tracer_core::infrastructure::crypto::internal
