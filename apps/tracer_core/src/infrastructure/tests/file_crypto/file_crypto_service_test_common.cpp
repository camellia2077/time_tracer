// infrastructure/tests/file_crypto/file_crypto_service_test_common.cpp
#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <iterator>

#include "infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp"

namespace android_runtime_tests::file_crypto_tests_internal {

auto Expect(bool condition, const std::string& message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto ReadTextFile(const std::filesystem::path& path) -> std::string {
  std::ifstream input(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

auto ReadBytes(const std::filesystem::path& path) -> std::vector<std::uint8_t> {
  std::ifstream input(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(input),
          std::istreambuf_iterator<char>()};
}

auto WriteBytes(const std::filesystem::path& path,
                const std::vector<std::uint8_t>& bytes) -> bool {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    return false;
  }
  if (!bytes.empty()) {
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
  }
  return output.good();
}

auto CountFilesByExtension(const std::filesystem::path& root,
                           std::string_view extension) -> std::size_t {
  if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
    return 0;
  }
  std::size_t count = 0;
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    if (entry.path().extension() == extension) {
      ++count;
    }
  }
  return count;
}

auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long> {
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(database, sql.c_str(), -1, &statement, nullptr) !=
          SQLITE_OK ||
      statement == nullptr) {
    return std::nullopt;
  }

  std::optional<long long> result;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    result = sqlite3_column_int64(statement, 0);
  }
  sqlite3_finalize(statement);
  return result;
}

auto ResolveRepoRootForInterop() -> std::filesystem::path {
  std::filesystem::path root = BuildRepoRoot();
  if (root.filename() == "apps") {
    root = root.parent_path();
  }
  return root;
}

void WriteU64LE(std::vector<std::uint8_t>& bytes, std::size_t offset,
                std::uint64_t value) {
  for (std::size_t i = 0; i < 8; ++i) {
    bytes[offset + i] = static_cast<std::uint8_t>((value >> (i * 8U)) & 0xFFU);
  }
}

auto UpdateBatchProgressExpectationState(
    BatchProgressExpectationState& state,
    const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot&
        snapshot) -> void {
  using tracer_core::infrastructure::crypto::FileCryptoPhase;

  state.has_scan = state.has_scan || snapshot.phase == FileCryptoPhase::kScan;
  state.has_read_input =
      state.has_read_input || snapshot.phase == FileCryptoPhase::kReadInput;
  state.has_completed =
      state.has_completed || snapshot.phase == FileCryptoPhase::kCompleted;
  state.has_group_count = state.has_group_count || snapshot.group_count == 2;

  if (snapshot.overall_done_bytes < state.previous_overall_done) {
    state.overall_monotonic = false;
  }

  const std::uint64_t expected_remaining =
      snapshot.overall_done_bytes < snapshot.overall_total_bytes
          ? snapshot.overall_total_bytes - snapshot.overall_done_bytes
          : 0;
  if (snapshot.remaining_bytes != expected_remaining) {
    state.remaining_bytes_consistent = false;
  }

  if (snapshot.speed_bytes_per_sec == 0) {
    if (snapshot.eta_seconds != 0) {
      state.eta_consistent = false;
    }
  } else {
    const std::uint64_t expected_eta =
        snapshot.remaining_bytes == 0
            ? 0
            : (snapshot.remaining_bytes + snapshot.speed_bytes_per_sec - 1) /
                  snapshot.speed_bytes_per_sec;
    if (snapshot.eta_seconds != expected_eta) {
      state.eta_consistent = false;
    }
  }

  if (snapshot.phase == FileCryptoPhase::kCompleted &&
      snapshot.remaining_bytes == 0 && snapshot.eta_seconds == 0) {
    state.completed_eta_cleared = true;
  }
  state.previous_overall_done = snapshot.overall_done_bytes;
}

}  // namespace android_runtime_tests::file_crypto_tests_internal
