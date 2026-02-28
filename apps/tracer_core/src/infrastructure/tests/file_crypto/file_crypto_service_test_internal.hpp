// infrastructure/tests/file_crypto/file_crypto_service_test_internal.hpp
#ifndef INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TEST_INTERNAL_HPP_
#define INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TEST_INTERNAL_HPP_

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/crypto/file_crypto_service.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

#if defined(_WIN32)
#if defined(EncryptFile)
#undef EncryptFile
#endif
#if defined(DecryptFile)
#undef DecryptFile
#endif
#endif

struct sqlite3;

namespace android_runtime_tests::file_crypto_tests_internal {

struct BatchProgressExpectationState {
  bool has_scan = false;
  bool has_read_input = false;
  bool has_completed = false;
  bool has_group_count = false;
  bool overall_monotonic = true;
  bool remaining_bytes_consistent = true;
  bool eta_consistent = true;
  bool completed_eta_cleared = false;
  std::uint64_t previous_overall_done = 0;
};

auto Expect(bool condition, const std::string& message, int& failures) -> void;

auto ReadTextFile(const std::filesystem::path& path) -> std::string;

auto ReadBytes(const std::filesystem::path& path) -> std::vector<std::uint8_t>;

auto WriteBytes(const std::filesystem::path& path,
                const std::vector<std::uint8_t>& bytes) -> bool;

auto CountFilesByExtension(const std::filesystem::path& root,
                           std::string_view extension) -> std::size_t;

auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long>;

auto ResolveRepoRootForInterop() -> std::filesystem::path;

void WriteU64LE(std::vector<std::uint8_t>& bytes, std::size_t offset,
                std::uint64_t value);

auto UpdateBatchProgressExpectationState(
    BatchProgressExpectationState& state,
    const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot&
        snapshot) -> void;

}  // namespace android_runtime_tests::file_crypto_tests_internal

namespace android_runtime_tests {

auto RunFileCryptoRoundtripTests(int& failures) -> void;
auto RunFileCryptoFailureTests(int& failures) -> void;
auto RunFileCryptoProgressTests(int& failures) -> void;
auto RunFileCryptoInteropTests(int& failures) -> void;

}  // namespace android_runtime_tests

#endif  // INFRASTRUCTURE_TESTS_FILE_CRYPTO_SERVICE_TEST_INTERNAL_HPP_
