// infrastructure/tests/android_runtime/android_runtime_report_consistency_support.cpp
#include <sodium.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <string_view>

#include "infrastructure/tests/android_runtime/android_runtime_report_consistency_internal.hpp"

#ifndef TT_ENABLE_HEAVY_DIAGNOSTICS
#define TT_ENABLE_HEAVY_DIAGNOSTICS 0
#endif

namespace android_runtime_tests::report_consistency_internal {
namespace {

auto ToHexLower(const unsigned char* bytes, size_t size) -> std::string {
  constexpr char kHex[] = "0123456789abcdef";
  std::string text(size * 2U, '\0');
  for (size_t index = 0; index < size; ++index) {
    const unsigned char value = bytes[index];
    text[index * 2U] = kHex[(value >> 4U) & 0x0FU];
    text[index * 2U + 1U] = kHex[value & 0x0FU];
  }
  return text;
}

#if TT_ENABLE_HEAVY_DIAGNOSTICS
auto BuildDiffContext(std::string_view text, size_t offset) -> std::string {
  constexpr size_t kRadius = 24;
  const size_t begin = (offset > kRadius) ? (offset - kRadius) : 0U;
  const size_t end = std::min(text.size(), offset + kRadius);
  std::string snippet(text.substr(begin, end - begin));
  for (char& value : snippet) {
    if (value == '\n' || value == '\r' || value == '\t') {
      value = ' ';
    }
  }
  return snippet;
}
#endif

}  // namespace

auto ComputeSha256Hex(std::string_view text) -> std::string {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return "sha256_error";
  }
  return ToHexLower(digest.data(), digest.size());
}

auto BuildDiffDiagnostics(std::string_view left, std::string_view right)
    -> std::string {
  const size_t max_len = std::min(left.size(), right.size());
  for (size_t index = 0; index < max_len; ++index) {
    if (left[index] == right[index]) {
      continue;
    }
    int line = 1;
    int column = 1;
    for (size_t pos = 0; pos < index; ++pos) {
      if (left[pos] == '\n') {
        ++line;
        column = 1;
      } else {
        ++column;
      }
    }

    std::ostringstream output;
    output << "first mismatch at line " << line << ", column " << column
           << ", byte offset " << index
           << "; left_sha256=" << ComputeSha256Hex(left)
           << ", right_sha256=" << ComputeSha256Hex(right);
#if TT_ENABLE_HEAVY_DIAGNOSTICS
    output << "; left_context=\"" << BuildDiffContext(left, index)
           << "\", right_context=\"" << BuildDiffContext(right, index) << "\"";
#endif
    return output.str();
  }

  if (left.size() != right.size()) {
    std::ostringstream output;
    output << "content length differs (left=" << left.size()
           << ", right=" << right.size()
           << "); left_sha256=" << ComputeSha256Hex(left)
           << ", right_sha256=" << ComputeSha256Hex(right);
    return output.str();
  }
  return "unknown mismatch.";
}

}  // namespace android_runtime_tests::report_consistency_internal
