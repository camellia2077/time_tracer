#include <iostream>
#include <stdexcept>

#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

auto main() -> int {
  using namespace tracer_core_c_api_stability_internal;

  try {
    LibHandle library = OpenLibrary(kLibraryName);
    if (library == nullptr) {
      throw std::runtime_error(std::string("Unable to load ") + kLibraryName +
                               ": " + LastDynamicError());
    }

    CoreApiFns api = LoadApi(library);
    const fs::path kRepoRoot = FindRepoRoot();
    Require(!kRepoRoot.empty(), "Unable to locate repository root");

    const fs::path kConverterConfig = kRepoRoot / "assets" / "tracer_core" /
                                      "config" / "converter" /
                                      "interval_processor_config.toml";
    const fs::path kInputRoot = kRepoRoot / "test" / "data";
    Require(fs::exists(kInputRoot), "Missing test/data directory");
    const fs::path kTempRoot =
        kRepoRoot / "test" / "output" / "tracer_core_c_api_query";
    const fs::path kOutputRoot = kTempRoot / "baseline";
    const fs::path kDbPath = kOutputRoot / "db" / "time_data.sqlite3";

    std::error_code io_error;
    fs::remove_all(kTempRoot, io_error);
    fs::create_directories(kOutputRoot, io_error);
    Require(!io_error, "Failed to prepare temp output directories");

    {
      auto runtime = CreateRuntime(api, kDbPath, kOutputRoot, kConverterConfig);
      SeedRuntimeWithBaselineData(api, runtime.Get(), kInputRoot);
      RunQueryChecks(api, runtime.Get());
    }

    std::cout << "[PASS] tracer_core_c_api_query_tests\n";
    CloseLibrary(library);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "[FAIL] tracer_core_c_api_query_tests: " << error.what()
              << '\n';
    return 1;
  }
}
