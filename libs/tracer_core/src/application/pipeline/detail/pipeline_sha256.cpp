#include "application/pipeline/detail/pipeline_sha256.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <span>
#include <sstream>
#include <vector>

namespace tracer::core::application::pipeline::detail {
namespace {

// SHA-256 helper formulas intentionally keep the specification's operand names
// and bit-shift constants for readability against the standard.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,readability-identifier-length)
[[nodiscard]] constexpr auto RotateRight(const std::uint32_t value,
                                         const std::uint32_t amount)
    -> std::uint32_t {
  return (value >> amount) | (value << (32U - amount));
}

[[nodiscard]] constexpr auto BigSigma0(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 2U) ^ RotateRight(value, 13U) ^
         RotateRight(value, 22U);
}

[[nodiscard]] constexpr auto BigSigma1(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 6U) ^ RotateRight(value, 11U) ^
         RotateRight(value, 25U);
}

[[nodiscard]] constexpr auto SmallSigma0(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 7U) ^ RotateRight(value, 18U) ^ (value >> 3U);
}

[[nodiscard]] constexpr auto SmallSigma1(const std::uint32_t value)
    -> std::uint32_t {
  return RotateRight(value, 17U) ^ RotateRight(value, 19U) ^ (value >> 10U);
}

[[nodiscard]] constexpr auto Choose(const std::uint32_t x,
                                    const std::uint32_t y,
                                    const std::uint32_t z) -> std::uint32_t {
  return (x & y) ^ (~x & z);
}
// NOLINTEND(readability-magic-numbers,readability-identifier-naming,readability-identifier-length)

[[nodiscard]] constexpr auto Majority(const std::uint32_t kX,
                                      const std::uint32_t kY,
                                      const std::uint32_t kZ)
    -> std::uint32_t {
  return (kX & kY) ^ (kX & kZ) ^ (kY & kZ);
}

[[nodiscard]] constexpr auto Sha256RoundConstants()
    -> std::array<std::uint32_t, 64> {
  // SHA-256 round constants are fixed by the algorithm specification.
  // NOLINTBEGIN(readability-magic-numbers)
  return {
      0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
      0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
      0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
      0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
      0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
      0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
      0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
      0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
      0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
      0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
      0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
      0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
      0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
  };
  // NOLINTEND(readability-magic-numbers)
}

[[nodiscard]] auto ComputeSha256Bytes(std::span<const std::uint8_t> data)
    -> std::array<std::uint8_t, 32> {
  static constexpr auto kRoundConstants = Sha256RoundConstants();
  std::array<std::uint32_t, 8> state = {
      0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
      0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
  };

  std::vector<std::uint8_t> padded(data.begin(), data.end());
  const std::uint64_t kBitLength =
      static_cast<std::uint64_t>(data.size()) * 8ULL;
  padded.push_back(0x80U);
  while ((padded.size() % 64U) != 56U) {
    padded.push_back(0x00U);
  }
  for (int shift = 56; shift >= 0; shift -= 8) {
    padded.push_back(
        static_cast<std::uint8_t>((kBitLength >> shift) & 0xFFULL));
  }

  std::array<std::uint32_t, 64> schedule{};
  for (std::size_t offset = 0; offset < padded.size(); offset += 64U) {
    for (std::size_t index = 0; index < 16U; ++index) {
      const std::size_t kBase = offset + index * 4U;
      schedule[index] =
          (static_cast<std::uint32_t>(padded[kBase]) << 24U) |
          (static_cast<std::uint32_t>(padded[kBase + 1U]) << 16U) |
          (static_cast<std::uint32_t>(padded[kBase + 2U]) << 8U) |
          static_cast<std::uint32_t>(padded[kBase + 3U]);
    }
    for (std::size_t index = 16U; index < 64U; ++index) {
      schedule[index] = SmallSigma1(schedule[index - 2U]) +
                        schedule[index - 7U] +
                        SmallSigma0(schedule[index - 15U]) +
                        schedule[index - 16U];
    }

    std::uint32_t a = state[0];
    std::uint32_t b = state[1];
    std::uint32_t c = state[2];
    std::uint32_t d = state[3];
    std::uint32_t e = state[4];
    std::uint32_t f = state[5];
    std::uint32_t g = state[6];
    std::uint32_t h = state[7];

    for (std::size_t index = 0; index < 64U; ++index) {
      const std::uint32_t kTemp1 =
          h + BigSigma1(e) + Choose(e, f, g) + kRoundConstants[index] +
          schedule[index];
      const std::uint32_t kTemp2 = BigSigma0(a) + Majority(a, b, c);
      h = g;
      g = f;
      f = e;
      e = d + kTemp1;
      d = c;
      c = b;
      b = a;
      a = kTemp1 + kTemp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
  }

  std::array<std::uint8_t, 32> digest{};
  for (std::size_t index = 0; index < state.size(); ++index) {
    const std::uint32_t kWord = state[index];
    digest[index * 4U] = static_cast<std::uint8_t>((kWord >> 24U) & 0xFFU);
    digest[index * 4U + 1U] =
        static_cast<std::uint8_t>((kWord >> 16U) & 0xFFU);
    digest[index * 4U + 2U] =
        static_cast<std::uint8_t>((kWord >> 8U) & 0xFFU);
    digest[index * 4U + 3U] = static_cast<std::uint8_t>(kWord & 0xFFU);
  }
  return digest;
}

}  // namespace

auto ComputeSha256Hex(std::string_view canonical_text) -> std::string {
  const auto* input =
      reinterpret_cast<const std::uint8_t*>(canonical_text.data());
  const auto kDigest = ComputeSha256Bytes(
      std::span<const std::uint8_t>(input, canonical_text.size()));

  std::ostringstream stream;
  stream << std::hex << std::setfill('0');
  for (const std::uint8_t kByte : kDigest) {
    stream << std::setw(2) << static_cast<unsigned int>(kByte);
  }
  return stream.str();
}

auto CurrentUnixMillis() -> std::int64_t {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

}  // namespace tracer::core::application::pipeline::detail
