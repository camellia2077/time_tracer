// api/core_c/tracer_core_c_api_runtime_tests.cpp
#include <atomic>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include "api/core_c/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {
namespace {

template <typename Fn>
auto RequireSymbol(LibHandle library, const char* symbol_name) -> Fn {
  void* symbol = LookupSymbol(library, symbol_name);
  if (symbol == nullptr) {
    throw std::runtime_error(std::string("Missing required symbol: ") +
                             symbol_name);
  }
  return reinterpret_cast<Fn>(symbol);
}

}  // namespace

#ifdef _WIN32
auto OpenLibrary(const char* library_name) -> LibHandle {
  return LoadLibraryA(library_name);
}

auto LookupSymbol(LibHandle handle, const char* symbol_name) -> void* {
  return reinterpret_cast<void*>(GetProcAddress(handle, symbol_name));
}

void CloseLibrary(LibHandle handle) {
  if (handle != nullptr) {
    FreeLibrary(handle);
  }
}

auto LastDynamicError() -> std::string {
  const DWORD kErrorCode = GetLastError();
  if (kErrorCode == 0U) {
    return "unknown";
  }
  return "Win32Error=" + std::to_string(static_cast<unsigned long>(kErrorCode));
}
#else
auto OpenLibrary(const char* library_name) -> LibHandle {
  return dlopen(library_name, RTLD_NOW);
}

auto LookupSymbol(LibHandle handle, const char* symbol_name) -> void* {
  return dlsym(handle, symbol_name);
}

void CloseLibrary(LibHandle handle) {
  if (handle != nullptr) {
    dlclose(handle);
  }
}

auto LastDynamicError() -> std::string {
  const char* error = dlerror();
  return error != nullptr ? std::string(error) : std::string("unknown");
}
#endif

RuntimeGuard::RuntimeGuard(RuntimeDestroyFn destroy_fn,
                           TtCoreRuntimeHandle* handle)
    : destroy_fn_(destroy_fn), handle_(handle) {}

RuntimeGuard::~RuntimeGuard() {
  if (handle_ != nullptr && destroy_fn_ != nullptr) {
    destroy_fn_(handle_);
    handle_ = nullptr;
  }
}

auto RuntimeGuard::Get() const -> TtCoreRuntimeHandle* {
  return handle_;
}

void Require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

auto LoadApi(LibHandle library) -> CoreApiFns {
  CoreApiFns api{};
  api.get_version =
      RequireSymbol<GetVersionFn>(library, "tracer_core_get_version");
  api.ping = RequireSymbol<PingFn>(library, "tracer_core_ping");
  api.get_capabilities = RequireSymbol<GetCapabilitiesFn>(
      library, "tracer_core_get_capabilities_json");
  api.last_error =
      RequireSymbol<LastErrorFn>(library, "tracer_core_last_error");
  api.runtime_check_environment = RequireSymbol<RuntimeCheckEnvironmentFn>(
      library, "tracer_core_runtime_check_environment_json");
  api.runtime_resolve_cli_context = RequireSymbol<RuntimeResolveCliContextFn>(
      library, "tracer_core_runtime_resolve_cli_context_json");
  api.runtime_create =
      RequireSymbol<RuntimeCreateFn>(library, "tracer_core_runtime_create");
  api.runtime_destroy =
      RequireSymbol<RuntimeDestroyFn>(library, "tracer_core_runtime_destroy");
  api.runtime_ingest = RequireSymbol<RuntimeIngestFn>(
      library, "tracer_core_runtime_ingest_json");
  api.runtime_convert = RequireSymbol<RuntimeConvertFn>(
      library, "tracer_core_runtime_convert_json");
  api.runtime_import = RequireSymbol<RuntimeImportFn>(
      library, "tracer_core_runtime_import_json");
  api.runtime_validate_structure = RequireSymbol<RuntimeValidateStructureFn>(
      library, "tracer_core_runtime_validate_structure_json");
  api.runtime_validate_logic = RequireSymbol<RuntimeValidateLogicFn>(
      library, "tracer_core_runtime_validate_logic_json");
  api.runtime_query =
      RequireSymbol<RuntimeQueryFn>(library, "tracer_core_runtime_query_json");
  api.runtime_report = RequireSymbol<RuntimeReportFn>(
      library, "tracer_core_runtime_report_json");
  api.runtime_report_batch = RequireSymbol<RuntimeReportBatchFn>(
      library, "tracer_core_runtime_report_batch_json");
  api.runtime_export = RequireSymbol<RuntimeExportFn>(
      library, "tracer_core_runtime_export_json");
  api.runtime_tree =
      RequireSymbol<RuntimeTreeFn>(library, "tracer_core_runtime_tree_json");
  return api;
}

auto FindRepoRoot() -> fs::path {
  fs::path current = fs::current_path();
  while (!current.empty()) {
    const fs::path kProbe = current / "apps" / "tracer_core" / "config" /
                            "converter" / "interval_processor_config.toml";
    if (fs::exists(kProbe)) {
      return current;
    }
    const fs::path kParent = current.parent_path();
    if (kParent == current) {
      break;
    }
    current = kParent;
  }
  return {};
}

auto ParseResponse(const char* response_json, std::string_view context)
    -> json {
  Require(response_json != nullptr && response_json[0] != '\0',
          std::string(context) + ": empty response");
  try {
    return json::parse(response_json);
  } catch (const std::exception& error) {
    throw std::runtime_error(std::string(context) +
                             ": invalid JSON response: " + error.what());
  }
}

void RequireOk(const char* response_json, std::string_view context) {
  const json kPayload = ParseResponse(response_json, context);
  const bool kIsOk = kPayload.value("ok", false);
  if (!kIsOk) {
    const std::string kErrorMessage = kPayload.value("error_message", "");
    throw std::runtime_error(
        std::string(context) +
        ": expected ok=true but got ok=false, error=" + kErrorMessage);
  }
}

void RequireNotOk(const char* response_json, std::string_view context) {
  const json kPayload = ParseResponse(response_json, context);
  const bool kIsOk = kPayload.value("ok", false);
  if (kIsOk) {
    throw std::runtime_error(std::string(context) +
                             ": expected ok=false but got ok=true");
  }
}

void RunCapabilitiesChecks(const CoreApiFns& api) {
  const json kCapabilities =
      ParseResponse(api.get_capabilities(), "capabilities");
  const auto kFeaturesIt = kCapabilities.find("features");
  Require(kFeaturesIt != kCapabilities.end() && kFeaturesIt->is_object(),
          "capabilities must contain object field `features`");

  const auto& features = *kFeaturesIt;
  const auto kRequireBool = [&](std::string_view key) -> void {
    const auto kFeatureIt = features.find(std::string(key));
    Require(kFeatureIt != features.end() && kFeatureIt->is_boolean(),
            "capabilities.features." + std::string(key) + " must be a boolean");
  };

  kRequireBool("runtime_ingest_json");
  kRequireBool("runtime_convert_json");
  kRequireBool("runtime_import_json");
  kRequireBool("runtime_validate_structure_json");
  kRequireBool("runtime_validate_logic_json");
  kRequireBool("runtime_query_json");
  kRequireBool("runtime_report_json");
  kRequireBool("runtime_report_batch_json");
  kRequireBool("runtime_export_json");
  kRequireBool("runtime_tree_json");
  kRequireBool("processed_json_io");
  kRequireBool("report_markdown");
  kRequireBool("report_latex");
  kRequireBool("report_typst");
}

void RunRuntimeBootstrapChecks(const CoreApiFns& api,
                               const fs::path& executable_path) {
  const json kRuntimeCheck = ParseResponse(
      api.runtime_check_environment(executable_path.string().c_str(), 0),
      "runtime_check_environment");
  Require(kRuntimeCheck.contains("ok") && kRuntimeCheck["ok"].is_boolean(),
          "runtime_check_environment response must contain boolean field `ok`");
  Require(kRuntimeCheck.contains("error_message") &&
              kRuntimeCheck["error_message"].is_string(),
          "runtime_check_environment response must contain string field "
          "`error_message`");
  if (kRuntimeCheck.contains("messages")) {
    Require(kRuntimeCheck["messages"].is_array(),
            "runtime_check_environment field `messages` must be array when "
            "present");
  }

  const json kResolvedContext = ParseResponse(
      api.runtime_resolve_cli_context(executable_path.string().c_str(), nullptr,
                                      nullptr, nullptr),
      "runtime_resolve_cli_context");
  Require(
      kResolvedContext.contains("ok") && kResolvedContext["ok"].is_boolean(),
      "runtime_resolve_cli_context response must contain boolean field "
      "`ok`");
  Require(kResolvedContext.contains("error_message") &&
              kResolvedContext["error_message"].is_string(),
          "runtime_resolve_cli_context response must contain string field "
          "`error_message`");

  if (kResolvedContext.value("ok", false)) {
    const auto kPathsIt = kResolvedContext.find("paths");
    Require(kPathsIt != kResolvedContext.end() && kPathsIt->is_object(),
            "runtime_resolve_cli_context ok=true requires object field "
            "`paths`");
    const auto kCliConfigIt = kResolvedContext.find("cli_config");
    Require(kCliConfigIt != kResolvedContext.end() && kCliConfigIt->is_object(),
            "runtime_resolve_cli_context ok=true requires object field "
            "`cli_config`");
  }
}

auto CreateRuntime(const CoreApiFns& api, const fs::path& db_path,
                   const fs::path& output_root,
                   const fs::path& converter_config) -> RuntimeGuard {
  std::error_code io_error;
  fs::create_directories(output_root, io_error);
  fs::create_directories(db_path.parent_path(), io_error);
  TtCoreRuntimeHandle* handle =
      api.runtime_create(db_path.string().c_str(), output_root.string().c_str(),
                         converter_config.string().c_str());
  if (handle == nullptr) {
    const char* error_message = api.last_error();
    const std::string kDetails =
        (error_message != nullptr && error_message[0] != '\0')
            ? std::string(error_message)
            : std::string("unknown");
    throw std::runtime_error("tracer_core_runtime_create failed: " + kDetails);
  }
  return {api.runtime_destroy, handle};
}

void RunConcurrentChecks(const CoreApiFns& api,
                         const RuntimePathBundle& paths) {
  static_cast<void>(paths.db_path);
  static_cast<void>(paths.temp_root);
  static_cast<void>(paths.converter_config);
  constexpr int kThreadCount = 8;
  constexpr int kIterationsPerThread = 40;

  std::mutex error_mutex;
  std::string first_error;
  std::atomic<bool> failed{false};

  auto worker = [&](int thread_index) -> void {
    try {
      static_cast<void>(thread_index);
      for (int iteration = 0; iteration < kIterationsPerThread; ++iteration) {
        const int kPingResult = api.ping();
        Require(kPingResult == TT_CORE_STATUS_OK,
                "concurrency ping should succeed");
        const char* version = api.get_version();
        Require(version != nullptr && version[0] != '\0',
                "concurrency version should be non-empty");
        RequireNotOk(api.runtime_query(
                         nullptr, json{{"action", "years"}}.dump().c_str()),
                     "concurrency invalid runtime query should fail");
      }
    } catch (const std::exception& error) {
      if (!failed.exchange(true)) {
        std::scoped_lock lock(error_mutex);
        first_error = error.what();
      }
    }
  };

  std::vector<std::thread> threads;
  threads.reserve(kThreadCount);
  for (int i = 0; i < kThreadCount; ++i) {
    threads.emplace_back(worker, i);
  }
  for (auto& thread : threads) {
    thread.join();
  }

  if (failed.load()) {
    throw std::runtime_error("Concurrent checks failed: " + first_error);
  }
}

void RunCreateDestroyChurn(const CoreApiFns& api,
                           const RuntimePathBundle& paths) {
  constexpr int kChurnIterations = 6;
  for (int i = 0; i < kChurnIterations; ++i) {
    const fs::path kOutputRoot =
        paths.temp_root / "churn" / ("iter_" + std::to_string(i));
    auto runtime =
        CreateRuntime(api, paths.db_path, kOutputRoot, paths.converter_config);
    Require(runtime.Get() != nullptr,
            "create-destroy churn runtime should be created");
  }
}

}  // namespace tracer_core_c_api_stability_internal

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

    const fs::path kConverterConfig = kRepoRoot / "apps" / "tracer_core" /
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
      RequireOk(api.runtime_ingest(baseline_runtime.Get(),
                                   json{{"input_path", kInputRoot.string()},
                                        {"date_check_mode", "none"},
                                        {"save_processed_output", false}}
                                       .dump()
                                       .c_str()),
                "baseline ingest");
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
