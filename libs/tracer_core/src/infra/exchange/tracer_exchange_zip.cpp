module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
#include <sodium.h>
#endif

module tracer.core.infrastructure.exchange;

namespace tracer::core::infrastructure::crypto::exchange {
namespace {

constexpr std::uint32_t kLocalHeader = 0x04034B50U;
constexpr std::uint32_t kCentralHeader = 0x02014B50U;
constexpr std::uint32_t kEndOfCentralDirectory = 0x06054B50U;
constexpr std::uint16_t kZipAesMethod = 99U;
constexpr std::uint16_t kDeflateMethod = 8U;
constexpr std::uint16_t kZipAesExtraId = 0x9901U;
constexpr std::uint16_t kUtf8Flag = 0x0800U;
constexpr std::uint16_t kEncryptedFlag = 0x0001U;
constexpr std::uint16_t kZipAesVersionNeeded = 51U;
constexpr std::size_t kZipAesSaltSize = 16U;
constexpr std::size_t kZipAesVerifierSize = 2U;
constexpr std::size_t kZipAesAuthCodeSize = 10U;
constexpr std::size_t kZipAesKeySize = 32U;
constexpr std::size_t kSha1Size = 20U;
constexpr std::size_t kSha1BlockSize = 64U;
constexpr std::uint16_t kZipAesVersion = 2U;
constexpr std::uint8_t kZipAesStrength256 = 3U;
constexpr std::uint32_t kPbkdf2Iterations = 1000U;

struct Sha1State {
  std::array<std::uint32_t, 5> words = {0x67452301U, 0xEFCDAB89U, 0x98BADCFEU,
                                        0x10325476U, 0xC3D2E1F0U};
  std::uint64_t message_size = 0U;
  std::array<std::uint8_t, kSha1BlockSize> buffer{};
  std::size_t buffered = 0U;
};

auto Rotl32(std::uint32_t value, unsigned count) -> std::uint32_t {
  return (value << count) | (value >> (32U - count));
}

auto ReadU16(std::span<const std::uint8_t> bytes, std::size_t offset)
    -> std::uint16_t {
  if (offset + 2U > bytes.size()) {
    throw std::runtime_error("ZIP header is truncated.");
  }
  return static_cast<std::uint16_t>(bytes[offset]) |
         (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
}

auto ReadU32(std::span<const std::uint8_t> bytes, std::size_t offset)
    -> std::uint32_t {
  if (offset + 4U > bytes.size()) {
    throw std::runtime_error("ZIP header is truncated.");
  }
  return static_cast<std::uint32_t>(bytes[offset]) |
         (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
         (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
         (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

auto ReadU64(std::span<const std::uint8_t> bytes, std::size_t offset)
    -> std::uint64_t {
  if (offset + 8U > bytes.size()) {
    throw std::runtime_error("ZIP header is truncated.");
  }
  std::uint64_t value = 0U;
  for (std::size_t index = 0U; index < 8U; ++index) {
    value |= static_cast<std::uint64_t>(bytes[offset + index]) << (index * 8U);
  }
  return value;
}

auto AppendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) -> void {
  bytes.push_back(static_cast<std::uint8_t>(value & 0xFFU));
  bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
}

auto AppendU32(std::vector<std::uint8_t>& bytes, std::uint32_t value) -> void {
  for (unsigned shift = 0U; shift < 32U; shift += 8U) {
    bytes.push_back(static_cast<std::uint8_t>((value >> shift) & 0xFFU));
  }
}

auto AppendU64(std::vector<std::uint8_t>& bytes, std::uint64_t value) -> void {
  for (unsigned shift = 0U; shift < 64U; shift += 8U) {
    bytes.push_back(static_cast<std::uint8_t>((value >> shift) & 0xFFU));
  }
}

auto Sha1Transform(Sha1State& state,
                   std::span<const std::uint8_t, kSha1BlockSize> block)
    -> void {
  std::array<std::uint32_t, 80> schedule{};
  for (std::size_t index = 0U; index < 16U; ++index) {
    schedule[index] =
        (static_cast<std::uint32_t>(block[index * 4U]) << 24U) |
        (static_cast<std::uint32_t>(block[index * 4U + 1U]) << 16U) |
        (static_cast<std::uint32_t>(block[index * 4U + 2U]) << 8U) |
        static_cast<std::uint32_t>(block[index * 4U + 3U]);
  }
  for (std::size_t index = 16U; index < schedule.size(); ++index) {
    schedule[index] = Rotl32(schedule[index - 3U] ^ schedule[index - 8U] ^
                                 schedule[index - 14U] ^ schedule[index - 16U],
                             1U);
  }

  std::uint32_t a = state.words[0];
  std::uint32_t b = state.words[1];
  std::uint32_t c = state.words[2];
  std::uint32_t d = state.words[3];
  std::uint32_t e = state.words[4];
  for (std::size_t index = 0U; index < schedule.size(); ++index) {
    std::uint32_t function = 0U;
    std::uint32_t constant = 0U;
    if (index < 20U) {
      function = (b & c) | ((~b) & d);
      constant = 0x5A827999U;
    } else if (index < 40U) {
      function = b ^ c ^ d;
      constant = 0x6ED9EBA1U;
    } else if (index < 60U) {
      function = (b & c) | (b & d) | (c & d);
      constant = 0x8F1BBCDCU;
    } else {
      function = b ^ c ^ d;
      constant = 0xCA62C1D6U;
    }
    const std::uint32_t kNext =
        Rotl32(a, 5U) + function + e + constant + schedule[index];
    e = d;
    d = c;
    c = Rotl32(b, 30U);
    b = a;
    a = kNext;
  }
  state.words[0] += a;
  state.words[1] += b;
  state.words[2] += c;
  state.words[3] += d;
  state.words[4] += e;
}

auto Sha1Update(Sha1State& state, std::span<const std::uint8_t> data) -> void {
  state.message_size += data.size();
  while (!data.empty()) {
    const std::size_t kCount =
        std::min(kSha1BlockSize - state.buffered, data.size());
    std::memcpy(state.buffer.data() + state.buffered, data.data(), kCount);
    state.buffered += kCount;
    data = data.subspan(kCount);
    if (state.buffered == kSha1BlockSize) {
      Sha1Transform(state, std::span<const std::uint8_t, kSha1BlockSize>(
                               state.buffer.data(), kSha1BlockSize));
      state.buffered = 0U;
    }
  }
}

auto Sha1Final(Sha1State state) -> std::array<std::uint8_t, kSha1Size> {
  const std::uint64_t kBitSize = state.message_size * 8U;
  const std::array<std::uint8_t, 1> kOne = {0x80U};
  Sha1Update(state, kOne);
  const std::array<std::uint8_t, 1> kZero = {0U};
  while (state.buffered != 56U) {
    Sha1Update(state, kZero);
  }
  std::array<std::uint8_t, 8> length{};
  for (std::size_t index = 0U; index < length.size(); ++index) {
    length[7U - index] =
        static_cast<std::uint8_t>((kBitSize >> (index * 8U)) & 0xFFU);
  }
  Sha1Update(state, length);

  std::array<std::uint8_t, kSha1Size> digest{};
  for (std::size_t index = 0U; index < state.words.size(); ++index) {
    for (unsigned shift = 0U; shift < 32U; shift += 8U) {
      digest[index * 4U + (3U - shift / 8U)] =
          static_cast<std::uint8_t>((state.words[index] >> shift) & 0xFFU);
    }
  }
  return digest;
}

auto Sha1(std::span<const std::uint8_t> data)
    -> std::array<std::uint8_t, kSha1Size> {
  Sha1State state{};
  Sha1Update(state, data);
  return Sha1Final(state);
}

struct HmacData {
  std::span<const std::uint8_t> bytes;
};

auto HmacSha1(std::span<const std::uint8_t> key, HmacData data)
    -> std::array<std::uint8_t, kSha1Size> {
  std::array<std::uint8_t, kSha1BlockSize> key_block{};
  if (key.size() > kSha1BlockSize) {
    const auto kDigest = Sha1(key);
    std::copy(kDigest.begin(), kDigest.end(), key_block.begin());
  } else {
    std::copy(key.begin(), key.end(), key_block.begin());
  }
  std::array<std::uint8_t, kSha1BlockSize> inner_pad{};
  std::array<std::uint8_t, kSha1BlockSize> outer_pad{};
  for (std::size_t index = 0U; index < kSha1BlockSize; ++index) {
    inner_pad[index] = key_block[index] ^ 0x36U;
    outer_pad[index] = key_block[index] ^ 0x5CU;
  }
  Sha1State inner{};
  Sha1Update(inner, inner_pad);
  Sha1Update(inner, data.bytes);
  const auto kInnerDigest = Sha1Final(inner);
  Sha1State outer{};
  Sha1Update(outer, outer_pad);
  Sha1Update(outer, kInnerDigest);
  return Sha1Final(outer);
}

auto DeriveZipAesKeys(std::string_view passphrase,
                      std::span<const std::uint8_t> salt)
    -> std::array<std::uint8_t, kZipAesKeySize * 2U + 2U> {
  std::array<std::uint8_t, kZipAesKeySize * 2U + 2U> derived{};
  std::vector<std::uint8_t> password(passphrase.begin(), passphrase.end());
  std::vector<std::uint8_t> block(salt.begin(), salt.end());
  AppendU32(block, 1U);
  auto u = HmacSha1(password, HmacData{block});
  std::copy(u.begin(), u.end(), derived.begin());
  for (std::uint32_t iteration = 1U; iteration < kPbkdf2Iterations;
       ++iteration) {
    u = HmacSha1(password, HmacData{u});
    for (std::size_t index = 0U; index < u.size(); ++index) {
      derived[index] ^= u[index];
    }
  }
  for (std::size_t block_number = 2U;
       (block_number - 1U) * kSha1Size < derived.size(); ++block_number) {
    block.assign(salt.begin(), salt.end());
    AppendU32(block, static_cast<std::uint32_t>(block_number));
    u = HmacSha1(password, HmacData{block});
    const std::size_t kOffset = (block_number - 1U) * kSha1Size;
    for (std::uint32_t iteration = 1U; iteration < kPbkdf2Iterations;
         ++iteration) {
      auto next = HmacSha1(password, HmacData{u});
      for (std::size_t index = 0U; index < u.size(); ++index) {
        u[index] ^= next[index];
      }
    }
    const std::size_t kCopySize = std::min(u.size(), derived.size() - kOffset);
    std::copy_n(u.begin(), kCopySize, derived.begin() + kOffset);
  }
  return derived;
}

constexpr std::array<std::uint8_t, 256> kAesSbox = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b,
    0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26,
    0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2,
    0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed,
    0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
    0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec,
    0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
    0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d,
    0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f,
    0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11,
    0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16};

auto Gmul2(std::uint8_t value) -> std::uint8_t {
  return static_cast<std::uint8_t>((value << 1U) ^
                                   ((value & 0x80U) != 0U ? 0x1BU : 0U));
}

auto AesEncryptBlock(std::span<const std::uint8_t, 16> input,
                     std::span<const std::uint8_t, 32> key)
    -> std::array<std::uint8_t, 16> {
  std::array<std::uint8_t, 240> round_keys{};
  std::copy(key.begin(), key.end(), round_keys.begin());
  std::uint8_t rcon = 1U;
  for (std::size_t offset = 32U; offset < round_keys.size(); offset += 4U) {
    std::array<std::uint8_t, 4> temp = {
        round_keys[offset - 4U], round_keys[offset - 3U],
        round_keys[offset - 2U], round_keys[offset - 1U]};
    if (offset % 32U == 0U) {
      const std::uint8_t kFirst = temp[0];
      temp[0] = kAesSbox[temp[1]] ^ rcon;
      temp[1] = kAesSbox[temp[2]];
      temp[2] = kAesSbox[temp[3]];
      temp[3] = kAesSbox[kFirst];
      rcon = Gmul2(rcon);
    } else if (offset % 32U == 16U) {
      for (auto& value : temp) {
        value = kAesSbox[value];
      }
    }
    for (std::size_t index = 0U; index < 4U; ++index) {
      round_keys[offset + index] =
          round_keys[offset - 32U + index] ^ temp[index];
    }
  }

  std::array<std::uint8_t, 16> state{};
  std::copy(input.begin(), input.end(), state.begin());
  auto add_round_key = [&](std::size_t offset) {
    for (std::size_t index = 0U; index < state.size(); ++index) {
      state[index] ^= round_keys[offset + index];
    }
  };
  auto sub_bytes = [&] {
    for (auto& value : state) {
      value = kAesSbox[value];
    }
  };
  auto shift_rows = [&] {
    const auto kOld = state;
    for (std::size_t column = 0U; column < 4U; ++column) {
      for (std::size_t row = 0U; row < 4U; ++row) {
        state[row + column * 4U] = kOld[row + ((column + row) % 4U) * 4U];
      }
    }
  };
  auto mix_columns = [&] {
    for (std::size_t column = 0U; column < 4U; ++column) {
      const std::size_t kBase = column * 4U;
      const auto kA = state[kBase];
      const auto kB = state[kBase + 1U];
      const auto kC = state[kBase + 2U];
      const auto kD = state[kBase + 3U];
      state[kBase] = Gmul2(kA) ^ static_cast<std::uint8_t>(kB ^ kC ^ kD);
      state[kBase + 1U] = Gmul2(kB) ^ static_cast<std::uint8_t>(kA ^ kC ^ kD);
      state[kBase + 2U] = Gmul2(kC) ^ static_cast<std::uint8_t>(kA ^ kB ^ kD);
      state[kBase + 3U] = Gmul2(kD) ^ static_cast<std::uint8_t>(kA ^ kB ^ kC);
    }
  };

  add_round_key(0U);
  for (std::size_t round = 1U; round < 14U; ++round) {
    sub_bytes();
    shift_rows();
    mix_columns();
    add_round_key(round * 16U);
  }
  sub_bytes();
  shift_rows();
  add_round_key(static_cast<std::size_t>(14U) * 16U);
  return state;
}

auto CryptAesCtr(std::span<const std::uint8_t> input,
                 std::span<const std::uint8_t, 32> key)
    -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> output(input.begin(), input.end());
  std::array<std::uint8_t, 16> counter{};
  counter[0] = 1U;
  for (std::size_t offset = 0U; offset < output.size(); offset += 16U) {
    const auto kStream = AesEncryptBlock(counter, key);
    const std::size_t kCount =
        std::min<std::size_t>(16U, output.size() - offset);
    for (std::size_t index = 0U; index < kCount; ++index) {
      output[offset + index] ^= kStream[index];
    }
    for (auto& value : counter) {
      if (++value != 0U) {
        break;
      }
    }
  }
  return output;
}

auto ConstantTimeEqual(std::span<const std::uint8_t> left,
                       std::span<const std::uint8_t> right) -> bool {
  if (left.size() != right.size()) {
    return false;
  }
  std::uint8_t difference = 0U;
  for (std::size_t index = 0U; index < left.size(); ++index) {
    difference |= left[index] ^ right[index];
  }
  return difference == 0U;
}

auto Crc32(std::span<const std::uint8_t> data) -> std::uint32_t {
  std::uint32_t crc = 0xFFFFFFFFU;
  for (const auto kByte : data) {
    crc ^= kByte;
    for (unsigned bit = 0U; bit < 8U; ++bit) {
      crc = (crc >> 1U) ^ (0xEDB88320U & (0U - (crc & 1U)));
    }
  }
  return ~crc;
}

auto DeflateStored(std::span<const std::uint8_t> data)
    -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> output;
  if (data.empty()) {
    output = {0x01U, 0x00U, 0x00U, 0xFFU, 0xFFU};
    return output;
  }
  std::size_t offset = 0U;
  while (offset < data.size()) {
    const std::size_t kCount =
        std::min<std::size_t>(65535U, data.size() - offset);
    const bool kFinal = offset + kCount == data.size();
    output.push_back(kFinal ? 0x01U : 0x00U);
    const auto kLength = static_cast<std::uint16_t>(kCount);
    AppendU16(output, kLength);
    AppendU16(output, static_cast<std::uint16_t>(~kLength));
    output.insert(
        output.end(),
        std::next(data.begin(), static_cast<std::ptrdiff_t>(offset)),
        std::next(data.begin(), static_cast<std::ptrdiff_t>(offset + kCount)));
    offset += kCount;
  }
  return output;
}

auto InflateStored(std::span<const std::uint8_t> data,
                   std::uint64_t expected_size) -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> output;
  std::size_t offset = 0U;
  bool final = false;
  while (!final) {
    if (offset + 5U > data.size()) {
      throw std::runtime_error("ZIP deflate stream is truncated.");
    }
    const std::uint8_t kHeader = data[offset++];
    final = (kHeader & 1U) != 0U;
    if (((kHeader >> 1U) & 3U) != 0U) {
      throw std::runtime_error(
          "ZIP deflate stream uses an unsupported compressed block.");
    }
    const std::uint16_t kLength = ReadU16(data, offset);
    const std::uint16_t kInverse = ReadU16(data, offset + 2U);
    offset += 4U;
    if (static_cast<std::uint16_t>(~kLength) != kInverse ||
        offset + kLength > data.size()) {
      throw std::runtime_error("ZIP deflate stored block is invalid.");
    }
    output.insert(
        output.end(),
        std::next(data.begin(), static_cast<std::ptrdiff_t>(offset)),
        std::next(data.begin(), static_cast<std::ptrdiff_t>(offset + kLength)));
    offset += kLength;
  }
  if (output.size() != expected_size) {
    throw std::runtime_error("ZIP uncompressed size does not match header.");
  }
  return output;
}

struct ZipEntryMeta {
  std::string path;
  std::uint32_t compressed_size = 0U;
  std::uint32_t uncompressed_size = 0U;
  std::uint32_t local_offset = 0U;
  std::uint16_t flags = 0U;
  std::uint16_t method = 0U;
  std::span<const std::uint8_t> extra;
};

auto BuildAesExtra() -> std::array<std::uint8_t, 11> {
  return {0x01U, 0x99U, 0x07U, 0x00U, 0x02U, 0x00U,
          'A',   'E',   0x03U, 0x08U, 0x00U};
}

auto EncryptEntry(std::span<const std::uint8_t> compressed,
                  std::string_view passphrase) -> std::vector<std::uint8_t> {
#if defined(TT_HAS_LIBSODIUM) && TT_HAS_LIBSODIUM
  std::array<std::uint8_t, kZipAesSaltSize> salt{};
  randombytes_buf(salt.data(), salt.size());
  const auto kKeys = DeriveZipAesKeys(passphrase, salt);
  const auto kEncrypted =
      CryptAesCtr(compressed, std::span<const std::uint8_t, kZipAesKeySize>(
                                  kKeys.data(), kZipAesKeySize));
  const auto kVerifier =
      std::span<const std::uint8_t, 2U>(kKeys.data() + kZipAesKeySize * 2U, 2U);
  const auto kAuth =
      HmacSha1(std::span<const std::uint8_t>(kKeys.data() + kZipAesKeySize,
                                             kZipAesKeySize),
               HmacData{kEncrypted});
  std::vector<std::uint8_t> output;
  output.insert(output.end(), salt.begin(), salt.end());
  output.insert(output.end(), kVerifier.begin(), kVerifier.end());
  output.insert(output.end(), kEncrypted.begin(), kEncrypted.end());
  output.insert(output.end(), kAuth.begin(),
                kAuth.begin() + kZipAesAuthCodeSize);
  return output;
#else
  (void)compressed;
  (void)passphrase;
  throw std::runtime_error("ZIP AES requires libsodium random bytes support.");
#endif
}

auto DecryptEntry(std::span<const std::uint8_t> encrypted,
                  std::string_view passphrase) -> std::vector<std::uint8_t> {
  if (encrypted.size() <
      kZipAesSaltSize + kZipAesVerifierSize + kZipAesAuthCodeSize) {
    throw std::runtime_error("ZIP AES entry is truncated.");
  }
  const auto kSalt = encrypted.subspan(0U, kZipAesSaltSize);
  const auto kKeys = DeriveZipAesKeys(passphrase, kSalt);
  const auto kVerifier =
      std::span<const std::uint8_t, 2U>(kKeys.data() + kZipAesKeySize * 2U, 2U);
  const auto kActualVerifier = encrypted.subspan(kZipAesSaltSize, 2U);
  if (!ConstantTimeEqual(kVerifier, kActualVerifier)) {
    throw std::runtime_error("ZIP AES password verification failed.");
  }
  const std::size_t kCipherOffset = kZipAesSaltSize + kZipAesVerifierSize;
  const std::size_t kCipherSize =
      encrypted.size() - kCipherOffset - kZipAesAuthCodeSize;
  const auto kCiphertext = encrypted.subspan(kCipherOffset, kCipherSize);
  const auto kExpectedAuth =
      HmacSha1(std::span<const std::uint8_t>(kKeys.data() + kZipAesKeySize,
                                             kZipAesKeySize),
               HmacData{kCiphertext});
  const auto kActualAuth =
      encrypted.subspan(encrypted.size() - kZipAesAuthCodeSize);
  if (!ConstantTimeEqual(std::span<const std::uint8_t>(kExpectedAuth.data(),
                                                       kZipAesAuthCodeSize),
                         kActualAuth)) {
    throw std::runtime_error("ZIP AES authentication failed.");
  }
  return CryptAesCtr(kCiphertext, std::span<const std::uint8_t, kZipAesKeySize>(
                                      kKeys.data(), kZipAesKeySize));
}

auto ValidatePath(std::string_view path) -> void {
  if (path.empty() || path.find('\\') != std::string_view::npos ||
      path.front() == '/' || path.find('\0') != std::string_view::npos) {
    throw std::runtime_error("ZIP entry path is invalid.");
  }
  std::size_t start = 0U;
  while (start < path.size()) {
    const std::size_t kEnd = path.find('/', start);
    const auto kComponent =
        path.substr(start, kEnd == std::string_view::npos ? path.size() - start
                                                          : kEnd - start);
    if (kComponent.empty() || kComponent == "." || kComponent == "..") {
      throw std::runtime_error("ZIP entry path may not escape its root.");
    }
    if (kEnd == std::string_view::npos) {
      break;
    }
    start = kEnd + 1U;
  }
}

auto FindAesExtra(std::span<const std::uint8_t> extra) -> std::uint16_t {
  std::size_t offset = 0U;
  while (offset + 4U <= extra.size()) {
    const std::uint16_t kId = ReadU16(extra, offset);
    const std::uint16_t kSize = ReadU16(extra, offset + 2U);
    offset += 4U;
    if (offset + kSize > extra.size()) {
      throw std::runtime_error("ZIP extra field is truncated.");
    }
    if (kId == kZipAesExtraId) {
      if (kSize != 7U || ReadU16(extra, offset) != kZipAesVersion ||
          extra[offset + 2U] != 'A' || extra[offset + 3U] != 'E' ||
          extra[offset + 4U] != kZipAesStrength256 ||
          ReadU16(extra, offset + 5U) != kDeflateMethod) {
        throw std::runtime_error("Unsupported ZIP AES extra field.");
      }
      return ReadU16(extra, offset + 5U);
    }
    offset += kSize;
  }
  throw std::runtime_error("ZIP AES extra field is missing.");
}

auto FindEndOfCentralDirectory(std::span<const std::uint8_t> bytes)
    -> std::size_t {
  const std::size_t kMinimum = 22U;
  if (bytes.size() < kMinimum) {
    throw std::runtime_error("ZIP end of central directory is missing.");
  }
  const std::size_t kBegin =
      bytes.size() > kMinimum + 65535U ? bytes.size() - kMinimum - 65535U : 0U;
  for (std::size_t offset = bytes.size() - kMinimum;; --offset) {
    if (ReadU32(bytes, offset) == kEndOfCentralDirectory) {
      return offset;
    }
    if (offset == kBegin) {
      break;
    }
  }
  throw std::runtime_error("ZIP end of central directory is missing.");
}

}  // namespace

auto EncodeZipBytes(const std::vector<TracerExchangePackageEntry>& entries,
                    std::string_view passphrase) -> std::vector<std::uint8_t> {
  if (passphrase.empty()) {
    throw std::runtime_error("ZIP AES passphrase must not be empty.");
  }
  // Reuse the v6 package validator. The ZIP is only a carrier; the logical
  // package contract remains the source of truth for entry ordering/content.
  static_cast<void>(EncodePackageBytes(entries));

  std::vector<std::uint8_t> archive;
  struct CentralRecord {
    std::string path;
    std::uint32_t compressed_size = 0U;
    std::uint32_t uncompressed_size = 0U;
    std::uint32_t local_offset = 0U;
  };
  std::vector<CentralRecord> central;
  central.reserve(entries.size());
  const auto kExtra = BuildAesExtra();

  for (const auto& entry : entries) {
    ValidatePath(entry.relative_path);
    if (entry.relative_path.size() >
            std::numeric_limits<std::uint16_t>::max() ||
        entry.data.size() > std::numeric_limits<std::uint32_t>::max()) {
      throw std::runtime_error("ZIP entry is too large.");
    }
    const auto kCompressed = DeflateStored(entry.data);
    const auto kEncrypted = EncryptEntry(kCompressed, passphrase);
    if (kEncrypted.size() > std::numeric_limits<std::uint32_t>::max() ||
        archive.size() > std::numeric_limits<std::uint32_t>::max()) {
      throw std::runtime_error("ZIP archive is too large.");
    }
    const auto kLocalOffset = static_cast<std::uint32_t>(archive.size());
    AppendU32(archive, kLocalHeader);
    AppendU16(archive, kZipAesVersionNeeded);
    AppendU16(archive, static_cast<std::uint16_t>(kEncryptedFlag | kUtf8Flag));
    AppendU16(archive, kZipAesMethod);
    AppendU16(archive, 0U);
    AppendU16(archive, 0U);
    AppendU32(archive, 0U);
    AppendU32(archive, static_cast<std::uint32_t>(kEncrypted.size()));
    AppendU32(archive, static_cast<std::uint32_t>(entry.data.size()));
    AppendU16(archive, static_cast<std::uint16_t>(entry.relative_path.size()));
    AppendU16(archive, static_cast<std::uint16_t>(kExtra.size()));
    archive.insert(archive.end(), entry.relative_path.begin(),
                   entry.relative_path.end());
    archive.insert(archive.end(), kExtra.begin(), kExtra.end());
    archive.insert(archive.end(), kEncrypted.begin(), kEncrypted.end());
    central.push_back(
        {entry.relative_path, static_cast<std::uint32_t>(kEncrypted.size()),
         static_cast<std::uint32_t>(entry.data.size()), kLocalOffset});
  }

  if (archive.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw std::runtime_error("ZIP central directory offset exceeds ZIP32.");
  }
  const auto kCentralOffset = static_cast<std::uint32_t>(archive.size());
  for (const auto& record : central) {
    AppendU32(archive, kCentralHeader);
    AppendU16(archive, 0x033FU);
    AppendU16(archive, kZipAesVersionNeeded);
    AppendU16(archive, static_cast<std::uint16_t>(kEncryptedFlag | kUtf8Flag));
    AppendU16(archive, kZipAesMethod);
    AppendU16(archive, 0U);
    AppendU16(archive, 0U);
    AppendU32(archive, 0U);
    AppendU32(archive, record.compressed_size);
    AppendU32(archive, record.uncompressed_size);
    AppendU16(archive, static_cast<std::uint16_t>(record.path.size()));
    AppendU16(archive, static_cast<std::uint16_t>(kExtra.size()));
    AppendU16(archive, 0U);
    AppendU16(archive, 0U);
    AppendU16(archive, 0U);
    AppendU32(archive, 0U);
    AppendU32(archive, record.local_offset);
    archive.insert(archive.end(), record.path.begin(), record.path.end());
    archive.insert(archive.end(), kExtra.begin(), kExtra.end());
  }
  const auto kCentralSize =
      static_cast<std::uint32_t>(archive.size()) - kCentralOffset;
  AppendU32(archive, kEndOfCentralDirectory);
  AppendU16(archive, 0U);
  AppendU16(archive, 0U);
  AppendU16(archive, static_cast<std::uint16_t>(central.size()));
  AppendU16(archive, static_cast<std::uint16_t>(central.size()));
  AppendU32(archive, kCentralSize);
  AppendU32(archive, kCentralOffset);
  AppendU16(archive, 0U);
  return archive;
}

auto DecodeZipBytes(std::span<const std::uint8_t> bytes,
                    std::string_view passphrase)
    -> DecodedTracerExchangePackage {
  if (passphrase.empty()) {
    throw std::runtime_error("ZIP AES passphrase must not be empty.");
  }
  const auto kEocd = FindEndOfCentralDirectory(bytes);
  const auto kDisk = ReadU16(bytes, kEocd + 4U);
  const auto kCentralDisk = ReadU16(bytes, kEocd + 6U);
  const auto kEntryCount = ReadU16(bytes, kEocd + 10U);
  const auto kCentralSize = ReadU32(bytes, kEocd + 12U);
  const auto kCentralOffset = ReadU32(bytes, kEocd + 16U);
  if (kDisk != 0U || kCentralDisk != 0U || kEntryCount == 0U ||
      static_cast<std::uint64_t>(kCentralOffset) + kCentralSize >
          bytes.size()) {
    throw std::runtime_error("Unsupported ZIP layout.");
  }

  std::vector<TracerExchangePackageEntry> entries;
  std::unordered_set<std::string> paths;
  std::size_t cursor = kCentralOffset;
  for (std::size_t index = 0U; index < kEntryCount; ++index) {
    if (ReadU32(bytes, cursor) != kCentralHeader ||
        cursor + 46U > bytes.size()) {
      throw std::runtime_error("ZIP central directory is invalid.");
    }
    const auto kFlags = ReadU16(bytes, cursor + 8U);
    const auto kMethod = ReadU16(bytes, cursor + 10U);
    const auto kCrc = ReadU32(bytes, cursor + 16U);
    const auto kCompressedSize = ReadU32(bytes, cursor + 20U);
    const auto kUncompressedSize = ReadU32(bytes, cursor + 24U);
    const auto kNameSize = ReadU16(bytes, cursor + 28U);
    const auto kExtraSize = ReadU16(bytes, cursor + 30U);
    const auto kCommentSize = ReadU16(bytes, cursor + 32U);
    const auto kLocalOffset = ReadU32(bytes, cursor + 42U);
    const std::size_t kRecordSize = 46U + kNameSize + kExtraSize + kCommentSize;
    if (cursor + kRecordSize > bytes.size() || kMethod != kZipAesMethod ||
        (kFlags & (kEncryptedFlag | kUtf8Flag)) !=
            (kEncryptedFlag | kUtf8Flag)) {
      throw std::runtime_error("ZIP entry is not a UTF-8 ZIP AES entry.");
    }
    const auto kNameBegin = cursor + 46U;
    std::string path(reinterpret_cast<const char*>(bytes.data() + kNameBegin),
                     kNameSize);
    ValidatePath(path);
    if (!paths.insert(path).second) {
      throw std::runtime_error("ZIP contains duplicate entry paths.");
    }
    const auto kExtraBegin = kNameBegin + kNameSize;
    const auto kActualMethod =
        FindAesExtra(bytes.subspan(kExtraBegin, kExtraSize));
    if (kActualMethod != kDeflateMethod) {
      throw std::runtime_error("ZIP entry compression method is unsupported.");
    }
    if (static_cast<std::uint64_t>(kLocalOffset) + 30U > bytes.size() ||
        ReadU32(bytes, kLocalOffset) != kLocalHeader) {
      throw std::runtime_error("ZIP local header is invalid.");
    }
    const auto kLocalNameSize = ReadU16(bytes, kLocalOffset + 26U);
    const auto kLocalExtraSize = ReadU16(bytes, kLocalOffset + 28U);
    const std::size_t kDataOffset = static_cast<std::size_t>(kLocalOffset) +
                                    30U + kLocalNameSize + kLocalExtraSize;
    if (kDataOffset > bytes.size() ||
        static_cast<std::uint64_t>(kDataOffset) + kCompressedSize >
            bytes.size()) {
      throw std::runtime_error("ZIP entry data is out of bounds.");
    }
    const auto kDecrypted =
        DecryptEntry(bytes.subspan(kDataOffset, kCompressedSize), passphrase);
    auto data = InflateStored(kDecrypted, kUncompressedSize);
    if (kCrc != 0U && Crc32(data) != kCrc) {
      throw std::runtime_error("ZIP entry CRC mismatch.");
    }
    TracerExchangePackageEntry entry{};
    entry.relative_path = std::move(path);
    entry.entry_flags = entry.relative_path.ends_with(".txt") ||
                                entry.relative_path.ends_with(".toml") ||
                                entry.relative_path == kManifestPath
                            ? kStandardEntryFlags
                            : kEntryFlagRequired;
    entry.data = std::move(data);
    entries.push_back(std::move(entry));
    cursor += kRecordSize;
  }

  if (cursor != static_cast<std::size_t>(kCentralOffset) + kCentralSize) {
    throw std::runtime_error("ZIP central directory size is inconsistent.");
  }
  // This validates manifest semantics, ordering, required paths and hashes
  // through the existing v6 package decoder.
  const auto kPackageBytes = EncodePackageBytes(entries);
  return DecodePackageBytes(kPackageBytes);
}

}  // namespace tracer::core::infrastructure::crypto::exchange
