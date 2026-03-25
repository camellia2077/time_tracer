#ifndef SHARED_UTILS_CANONICAL_TEXT_H_
#define SHARED_UTILS_CANONICAL_TEXT_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace tracer::core::shared::canonical_text {

struct CanonicalizationResult {
  bool ok = false;
  std::string text;
  std::string error_message;
};

namespace detail {

inline auto BuildSourceLabel(std::string_view source_label) -> std::string {
  return source_label.empty() ? std::string("text input")
                              : std::string(source_label);
}

inline auto BuildUtf8Error(std::string_view source_label,
                           std::string_view details) -> std::string {
  return "Invalid UTF-8 in " + BuildSourceLabel(source_label) + ": " +
         std::string(details);
}

inline auto IsContinuationByte(std::uint8_t value) -> bool {
  return (value & 0xC0U) == 0x80U;
}

inline auto ValidateUtf8(std::span<const std::uint8_t> bytes,
                         std::string_view source_label) -> std::string {
  const auto error_at = [&](std::size_t offset,
                            std::string_view details) -> std::string {
    return BuildUtf8Error(source_label,
                          details.empty()
                              ? ("at byte " + std::to_string(offset + 1))
                              : (std::string(details) + " at byte " +
                                 std::to_string(offset + 1)));
  };

  std::size_t index = 0;
  while (index < bytes.size()) {
    const std::uint8_t lead = bytes[index];
    if (lead <= 0x7FU) {
      ++index;
      continue;
    }

    const auto require_continuation = [&](std::size_t offset) -> bool {
      return offset < bytes.size() && IsContinuationByte(bytes[offset]);
    };

    if (lead >= 0xC2U && lead <= 0xDFU) {
      if (!require_continuation(index + 1)) {
        return error_at(index, "truncated or invalid 2-byte sequence");
      }
      index += 2;
      continue;
    }

    if (lead == 0xE0U) {
      if (index + 2 >= bytes.size()) {
        return error_at(index, "truncated 3-byte sequence");
      }
      if (bytes[index + 1] < 0xA0U || bytes[index + 1] > 0xBFU ||
          !require_continuation(index + 2)) {
        return error_at(index, "invalid 3-byte sequence");
      }
      index += 3;
      continue;
    }

    if ((lead >= 0xE1U && lead <= 0xECU) || (lead >= 0xEEU && lead <= 0xEFU)) {
      if (index + 2 >= bytes.size()) {
        return error_at(index, "truncated 3-byte sequence");
      }
      if (!require_continuation(index + 1) ||
          !require_continuation(index + 2)) {
        return error_at(index, "invalid 3-byte sequence");
      }
      index += 3;
      continue;
    }

    if (lead == 0xEDU) {
      if (index + 2 >= bytes.size()) {
        return error_at(index, "truncated 3-byte sequence");
      }
      if (bytes[index + 1] < 0x80U || bytes[index + 1] > 0x9FU ||
          !require_continuation(index + 2)) {
        return error_at(index,
                        "UTF-16 surrogate code points are not valid UTF-8");
      }
      index += 3;
      continue;
    }

    if (lead == 0xF0U) {
      if (index + 3 >= bytes.size()) {
        return error_at(index, "truncated 4-byte sequence");
      }
      if (bytes[index + 1] < 0x90U || bytes[index + 1] > 0xBFU ||
          !require_continuation(index + 2) ||
          !require_continuation(index + 3)) {
        return error_at(index, "invalid 4-byte sequence");
      }
      index += 4;
      continue;
    }

    if (lead >= 0xF1U && lead <= 0xF3U) {
      if (index + 3 >= bytes.size()) {
        return error_at(index, "truncated 4-byte sequence");
      }
      if (!require_continuation(index + 1) ||
          !require_continuation(index + 2) ||
          !require_continuation(index + 3)) {
        return error_at(index, "invalid 4-byte sequence");
      }
      index += 4;
      continue;
    }

    if (lead == 0xF4U) {
      if (index + 3 >= bytes.size()) {
        return error_at(index, "truncated 4-byte sequence");
      }
      if (bytes[index + 1] < 0x80U || bytes[index + 1] > 0x8FU ||
          !require_continuation(index + 2) ||
          !require_continuation(index + 3)) {
        return error_at(index,
                        "code points above U+10FFFF are not valid UTF-8");
      }
      index += 4;
      continue;
    }

    return error_at(index, "invalid UTF-8 leading byte");
  }

  return "";
}

inline auto NormalizeValidatedBytes(std::span<const std::uint8_t> bytes)
    -> std::string {
  constexpr std::uint8_t kBom0 = 0xEFU;
  constexpr std::uint8_t kBom1 = 0xBBU;
  constexpr std::uint8_t kBom2 = 0xBFU;
  std::size_t start = 0;
  if (bytes.size() >= 3 && bytes[0] == kBom0 && bytes[1] == kBom1 &&
      bytes[2] == kBom2) {
    start = 3;
  }

  std::string normalized;
  normalized.reserve(bytes.size() - start);
  for (std::size_t index = start; index < bytes.size(); ++index) {
    const char ch = static_cast<char>(bytes[index]);
    if (ch == '\r') {
      normalized.push_back('\n');
      if (index + 1 < bytes.size() &&
          static_cast<char>(bytes[index + 1]) == '\n') {
        ++index;
      }
      continue;
    }
    normalized.push_back(ch);
  }
  return normalized;
}

inline auto CanonicalizeValidatedBytes(std::span<const std::uint8_t> bytes,
                                       std::string_view source_label)
    -> CanonicalizationResult {
  const std::string validation_error = ValidateUtf8(bytes, source_label);
  if (!validation_error.empty()) {
    return {.ok = false, .text = "", .error_message = validation_error};
  }
  return {
      .ok = true, .text = NormalizeValidatedBytes(bytes), .error_message = ""};
}

}  // namespace detail

inline auto Canonicalize(std::span<const std::uint8_t> bytes,
                         std::string_view source_label = {})
    -> CanonicalizationResult {
  return detail::CanonicalizeValidatedBytes(bytes, source_label);
}

inline auto Canonicalize(std::string_view text,
                         std::string_view source_label = {})
    -> CanonicalizationResult {
  const auto* begin =
      reinterpret_cast<const std::uint8_t*>(text.data());  // NOLINT
  return detail::CanonicalizeValidatedBytes(
      std::span<const std::uint8_t>(begin, text.size()), source_label);
}

inline auto RequireCanonicalText(std::span<const std::uint8_t> bytes,
                                 std::string_view source_label = {})
    -> std::string {
  const CanonicalizationResult result = Canonicalize(bytes, source_label);
  if (!result.ok) {
    throw std::runtime_error(result.error_message);
  }
  return result.text;
}

inline auto RequireCanonicalText(std::string_view text,
                                 std::string_view source_label = {})
    -> std::string {
  const CanonicalizationResult result = Canonicalize(text, source_label);
  if (!result.ok) {
    throw std::runtime_error(result.error_message);
  }
  return result.text;
}

inline auto ToUtf8Bytes(std::string_view canonical_text)
    -> std::vector<std::uint8_t> {
  return {canonical_text.begin(), canonical_text.end()};
}

}  // namespace tracer::core::shared::canonical_text

#endif  // SHARED_UTILS_CANONICAL_TEXT_H_
