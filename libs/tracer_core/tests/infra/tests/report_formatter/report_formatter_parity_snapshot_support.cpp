// infrastructure/tests/report_formatter/report_formatter_parity_snapshot_support.cpp
#include <sodium.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "infra/tests/report_formatter/report_formatter_parity_internal.hpp"

#ifndef TT_ENABLE_HEAVY_DIAGNOSTICS
#define TT_ENABLE_HEAVY_DIAGNOSTICS 0
#endif

namespace report_formatter_parity_internal {
namespace {

auto ToHexLower(const unsigned char* bytes, size_t size) -> std::string {
  constexpr std::array<char, 16> kHex = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  };
  constexpr unsigned kHexShiftBits = 4U;
  constexpr std::uint8_t kHexNibbleMask = 0x0FU;
  std::string text(size * 2U, '\0');
  for (size_t index = 0; index < size; ++index) {
    const unsigned char kValue = bytes[index];
    text[index * 2U] = kHex[(kValue >> kHexShiftBits) & kHexNibbleMask];
    text[(index * 2U) + 1U] = kHex[kValue & kHexNibbleMask];
  }
  return text;
}

auto ComputeSha256Hex(std::string_view text) -> std::string {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return "sha256_error";
  }
  return ToHexLower(digest.data(), digest.size());
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

auto ReadFileText(const std::filesystem::path& path)
    -> std::optional<std::string> {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  return content;
}

auto WriteFileText(const std::filesystem::path& path,
                   const std::string& content) -> bool {
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  if (error) {
    return false;
  }

  std::ofstream output(path, std::ios::trunc | std::ios::binary);
  if (!output.is_open()) {
    return false;
  }
  output.write(content.data(), static_cast<std::streamsize>(content.size()));
  return static_cast<bool>(output);
}

auto BuildFirstDiffMessage(const std::string& left, const std::string& right)
    -> std::string {
  const size_t kMaxLength = std::min(left.size(), right.size());
  for (size_t index = 0; index < kMaxLength; ++index) {
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
           << ", byte offset " << index << "; "
           << "left_sha256=" << ComputeSha256Hex(left) << ", "
           << "right_sha256=" << ComputeSha256Hex(right) << ".";
#if TT_ENABLE_HEAVY_DIAGNOSTICS
    output << " left_context=\"" << BuildDiffContext(left, index)
           << "\", right_context=\"" << BuildDiffContext(right, index) << "\".";
#endif
    return output.str();
  }

  if (left.size() != right.size()) {
    std::ostringstream output;
    output << "content length differs (expected " << left.size() << ", actual "
           << right.size() << "); "
           << "left_sha256=" << ComputeSha256Hex(left) << ", "
           << "right_sha256=" << ComputeSha256Hex(right) << ".";
    return output.str();
  }
  return "unknown mismatch.";
}

auto AssertParityAndSnapshot(const std::string& case_name,
                             const std::string& snapshot_content,
                             const std::string& cli_content,
                             const std::string& android_content, int& failures)
    -> void {
  if (cli_content != android_content) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " parity failed between CLI and Android: "
              << BuildFirstDiffMessage(cli_content, android_content) << '\n';
  }

  if (snapshot_content != cli_content) {
    ++failures;
    std::cerr << "[FAIL] " << case_name << " snapshot mismatch: "
              << BuildFirstDiffMessage(snapshot_content, cli_content) << '\n';
  }
}

auto RunCaseWithSnapshot(const std::string& case_name,
                         const std::filesystem::path& snapshot_file,
                         const std::string& cli_output,
                         const std::string& android_output,
                         bool update_snapshots, int& failures) -> void {
  if (cli_output != android_output) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " raw-byte parity failed between CLI and Android: "
              << BuildFirstDiffMessage(cli_output, android_output) << '\n';
  }

  const std::string kCliHash = ComputeSha256Hex(cli_output);
  const std::string kAndroidHash = ComputeSha256Hex(android_output);
  if (kCliHash != kAndroidHash) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " hash parity failed between CLI and Android: cli_sha256="
              << kCliHash << ", android_sha256=" << kAndroidHash << '\n';
  }

  const std::string kCliSnapshotText = cli_output;
  const std::string kAndroidSnapshotText = android_output;

  if (update_snapshots) {
    if (!WriteFileText(snapshot_file, kCliSnapshotText)) {
      ++failures;
      std::cerr << "[FAIL] " << case_name
                << " failed to update snapshot: " << snapshot_file << '\n';
    }
    return;
  }

  const auto kSnapshot = ReadFileText(snapshot_file);
  if (!kSnapshot.has_value()) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " missing snapshot file: " << snapshot_file
              << " (run with TT_UPDATE_FORMATTER_SNAPSHOTS=1)\n";
    return;
  }

  AssertParityAndSnapshot(case_name, *kSnapshot, kCliSnapshotText,
                          kAndroidSnapshotText, failures);
}

auto RunFormatSnapshotCases(const std::string& format_label,
                            const std::string& extension,
                            const std::filesystem::path& snapshot_root,
                            const CaseOutputs& cli_outputs,
                            const CaseOutputs& android_outputs,
                            bool update_snapshots, int& failures) -> void {
  RunCaseWithSnapshot("daily/" + format_label,
                      snapshot_root / ("day" + extension), cli_outputs.day,
                      android_outputs.day, update_snapshots, failures);
  RunCaseWithSnapshot("monthly/" + format_label,
                      snapshot_root / ("month" + extension), cli_outputs.month,
                      android_outputs.month, update_snapshots, failures);
  RunCaseWithSnapshot("weekly/" + format_label,
                      snapshot_root / ("week" + extension), cli_outputs.week,
                      android_outputs.week, update_snapshots, failures);
  RunCaseWithSnapshot("yearly/" + format_label,
                      snapshot_root / ("year" + extension), cli_outputs.year,
                      android_outputs.year, update_snapshots, failures);
  RunCaseWithSnapshot("range/" + format_label,
                      snapshot_root / ("range" + extension), cli_outputs.range,
                      android_outputs.range, update_snapshots, failures);
}

}  // namespace report_formatter_parity_internal
