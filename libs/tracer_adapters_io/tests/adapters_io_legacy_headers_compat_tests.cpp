import tracer.adapters.io;

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

#include "infrastructure/io/file_io_service.hpp"
#include "infrastructure/io/processed_data_io.hpp"
#include "infrastructure/io/txt_ingest_input_provider.hpp"

namespace {

namespace fs = std::filesystem;

using tracer::adapters::io::modcore::CreateDirectories;
using tracer::adapters::io::modcore::WriteContent;
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
         ("tt_adapters_io_legacy_compat_" + std::to_string(now));
}

void TestLegacyHeadersRemainUsable(int& failures) {
  static_assert(std::is_base_of_v<tracer_core::application::ports::IIngestInputProvider,
                                  infrastructure::io::TxtIngestInputProvider>);
  static_assert(std::is_base_of_v<tracer_core::application::ports::IProcessedDataLoader,
                                  infrastructure::io::ProcessedDataLoader>);
  static_assert(
      std::is_base_of_v<tracer_core::application::ports::IProcessedDataStorage,
                        infrastructure::io::ProcessedDataStorage>);

  const fs::path root = MakeWorkspaceRoot();
  const fs::path logs = root / "logs";
  const fs::path note = logs / "a.txt";

  try {
    CreateDirectories(logs);
    WriteContent(note, "legacy header compat");

    // Legacy class API remains stable for existing call sites.
    FileIoService::PrepareOutputDirectories(root / "output");
    const auto files = FileIoService::FindLogFiles(root);
    Expect(files.size() == 1U,
           "Legacy FileIoService::FindLogFiles should still return txt files.",
           failures);

    infrastructure::io::TxtIngestInputProvider provider;
    const auto collected = provider.CollectTextInputs(root, ".txt");
    Expect(collected.input_exists, "Legacy provider should observe existing input root.",
           failures);
    Expect(collected.inputs.size() == 1U,
           "Legacy provider should still collect txt payloads.", failures);

    const auto resolved = ResolveFiles({root.string()}, ".txt");
    Expect(resolved.size() == 1U,
           "Module helper should coexist with legacy headers.", failures);

    infrastructure::io::ProcessedDataLoader loader;
    infrastructure::io::ProcessedDataStorage storage;
    (void)loader;
    (void)storage;
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
  TestLegacyHeadersRemainUsable(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_adapters_io_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_adapters_io_legacy_headers_compat_tests failures: "
            << failures << '\n';
  return 1;
}
