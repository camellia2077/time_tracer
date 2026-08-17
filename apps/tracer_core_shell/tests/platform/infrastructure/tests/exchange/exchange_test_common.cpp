#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <iterator>

#include "infrastructure/tests/exchange/exchange_test_common.hpp"

namespace android_runtime_tests::exchange_tests_internal {

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
    if (entry.is_regular_file() && entry.path().extension() == extension) {
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

}  // namespace android_runtime_tests::exchange_tests_internal
