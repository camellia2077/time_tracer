import tracer.adapters.io;

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace fs = std::filesystem;

using tracer::adapters::io::modcore::CreateDirectories;
using tracer::adapters::io::modcore::Exists;
using tracer::adapters::io::modcore::IsDirectory;
using tracer::adapters::io::modcore::IsRegularFile;
using tracer::adapters::io::modcore::ReadContent;
using tracer::adapters::io::modcore::WriteContent;
using tracer::adapters::io::modutils::FindFilesByExtensionRecursively;
using tracer::adapters::io::modutils::ResolveFiles;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto MakeWorkspaceRoot() -> fs::path {
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return fs::temp_directory_path() /
         ("tt_adapters_io_modules_smoke_" + std::to_string(now));
}

void TestCoreAndUtilsBridge(int& failures) {
  const fs::path root = MakeWorkspaceRoot();
  const fs::path nested = root / "nested";
  const fs::path note = nested / "note.txt";
  const fs::path second = nested / "more.txt";
  const fs::path ignored = nested / "ignore.log";

  try {
    CreateDirectories(nested);
    WriteContent(note, "hello modules");
    WriteContent(second, "second line");
    WriteContent(ignored, "ignored");

    Expect(Exists(root), "Root directory should exist.", failures);
    Expect(IsDirectory(nested), "Nested path should be directory.", failures);
    Expect(IsRegularFile(note), "note.txt should be a regular file.", failures);
    Expect(ReadContent(note) == "hello modules", "ReadContent result mismatch.",
           failures);

    const auto files = FindFilesByExtensionRecursively(root, ".txt");
    Expect(files.size() == 2U, "FindFilesByExtensionRecursively size mismatch.",
           failures);

    const std::vector<std::string> input_paths = {root.string(), note.string()};
    const auto resolved = ResolveFiles(input_paths, ".txt");
    Expect(resolved.size() == 2U, "ResolveFiles should deduplicate entries.",
           failures);
  } catch (const std::exception& ex) {
    ++failures;
    std::cerr << "[FAIL] Unexpected exception: " << ex.what() << '\n';
  }

  std::error_code ec;
  (void)fs::remove_all(root, ec);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestCoreAndUtilsBridge(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_adapters_io_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_adapters_io_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
