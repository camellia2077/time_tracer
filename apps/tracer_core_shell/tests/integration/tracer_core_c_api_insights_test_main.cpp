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

    const fs::path kFixtureRoot =
        kRepoRoot / "test" / "fixtures" / "text" / "insights";
    const fs::path kFixtureConfigRoot =
        kRepoRoot / "test" / "fixtures" / "config" / "insights";
    const fs::path kInputRoot = kFixtureRoot;
    Require(fs::exists(kInputRoot), "Missing insights fixture input directory");
    Require(fs::exists(kFixtureConfigRoot), "Missing insights fixture config");
    const fs::path kTempRoot =
        kRepoRoot / "test" / "output" / "tracer_core_c_api_insights";
    const fs::path kOutputRoot = kTempRoot / "baseline";
    const fs::path kDbPath = kOutputRoot / "db" / "time_data.sqlite3";

    std::error_code io_error;
    fs::remove_all(kTempRoot, io_error);
    fs::create_directories(kOutputRoot, io_error);
    Require(!io_error, "Failed to prepare temp output directories");

    const fs::path kRuntimeConfigRoot = kTempRoot / "config";
    const fs::path kRuntimeProgramRoot = kRuntimeConfigRoot / "program";
    const fs::path kRuntimeUserRoot = kRuntimeConfigRoot / "user";
    fs::create_directories(kRuntimeConfigRoot, io_error);
    Require(!io_error, "Failed to create insights fixture config root");
    fs::copy(kRepoRoot / "config" / "program", kRuntimeProgramRoot,
             fs::copy_options::recursive | fs::copy_options::overwrite_existing,
             io_error);
    Require(!io_error,
            "Failed to copy shared program config for insights fixture");
    fs::create_directories(kRuntimeUserRoot / "activity_hierarchy", io_error);
    Require(!io_error, "Failed to create insights fixture user config");
    fs::copy_file(
        kFixtureConfigRoot / "behavior.toml",
        kRuntimeUserRoot / "behavior.toml",
        fs::copy_options::overwrite_existing,
        io_error);
    Require(!io_error, "Failed to install insights fixture behavior config");
    fs::copy_file(
        kRepoRoot / "config" / "user" / "insights.toml",
        kRuntimeUserRoot / "insights.toml",
        fs::copy_options::overwrite_existing,
        io_error);
    Require(!io_error, "Failed to install insights fixture insights config");
    fs::copy_file(
        kFixtureConfigRoot / "activity_hierarchy" / "insights.toml",
        kRuntimeUserRoot / "activity_hierarchy" / "insights.toml",
        fs::copy_options::overwrite_existing,
        io_error);
    Require(!io_error, "Failed to install insights fixture activity config");

    const fs::path kConverterConfig = kRuntimeUserRoot / "behavior.toml";

    {
      auto runtime = CreateRuntime(api, kDbPath, kOutputRoot, kConverterConfig);
      SeedRuntimeWithBaselineData(api, runtime.Get(), kInputRoot);
      RunInsightsChecks(api, runtime.Get(), kOutputRoot);
    }

    std::cout << "[PASS] tracer_core_c_api_insights_tests\n";
    CloseLibrary(library);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "[FAIL] tracer_core_c_api_insights_tests: " << error.what()
              << '\n';
    return 1;
  }
}
