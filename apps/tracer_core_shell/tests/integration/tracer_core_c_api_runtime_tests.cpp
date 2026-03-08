// tests/integration/tracer_core_c_api_runtime_tests.cpp
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
    Require(api.ping() == TT_CORE_STATUS_OK,
            "tracer_core_ping failed in stability tests");
    const char* version = api.get_version();
    Require(version != nullptr && version[0] != '\0',
            "tracer_core_get_version returned empty string");
    RunCapabilitiesChecks(api);

    const fs::path kRepoRoot = FindRepoRoot();
    Require(!kRepoRoot.empty(),
            "Unable to locate repository root in stability tests");

    const fs::path kConverterConfig = kRepoRoot / "assets" / "tracer_core" /
                                      "config" / "converter" /
                                      "interval_processor_config.toml";
    fs::path cli_executable = kRepoRoot / "apps" / "tracer_cli" / "windows" /
                              "build_fast" / "bin" / "time_tracer_cli.exe";
    if (!fs::exists(cli_executable)) {
      cli_executable = kRepoRoot / "apps" / "tracer_cli" / "windows" / "build" /
                       "bin" / "time_tracer_cli.exe";
    }
    const fs::path kInputRoot = kRepoRoot / "test" / "data";
    Require(fs::exists(kInputRoot), "Missing test/data directory");

    const fs::path kTempRoot =
        kRepoRoot / "test" / "output" / "tracer_core_c_api_stability";
    const fs::path kOutputRoot = kTempRoot / "baseline";
    const fs::path kDbPath = kOutputRoot / "db" / "time_data.sqlite3";
    const RuntimePathBundle kRuntimePaths{
        .db_path = kDbPath,
        .temp_root = kTempRoot,
        .converter_config = kConverterConfig,
    };

    std::error_code io_error;
    fs::remove_all(kTempRoot, io_error);
    fs::create_directories(kOutputRoot, io_error);
    Require(!io_error, "Failed to prepare temp output directories");
    RunRuntimeBootstrapChecks(api, cli_executable);

    {
      auto baseline_runtime =
          CreateRuntime(api, kDbPath, kOutputRoot, kConverterConfig);
      RunCallbackBridgeChecks(api, baseline_runtime.Get(), kInputRoot);
      RunQueryChecks(api, baseline_runtime.Get());
      RunErrorPathChecks(api, baseline_runtime.Get(), kConverterConfig);
      RunConcurrentChecks(api, kRuntimePaths);
      RunCreateDestroyChurn(api, kRuntimePaths);
    }

    std::cout << "[PASS] tracer_core_c_api_stability_tests version=" << version
              << '\n';
    CloseLibrary(library);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "[FAIL] tracer_core_c_api_stability_tests: " << error.what()
              << '\n';
    return 1;
  }
}
