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

struct CallbackProbeState {
  std::atomic<int> kLogCount{0};
  std::atomic<int> kDiagnosticsCount{0};
  std::atomic<int> kCryptoProgressCount{0};
};

extern "C" void CaptureLogCallback(TtCoreLogSeverity /*severity*/,
                                   const char* utf8_message,
                                   void* user_data) {
  if (user_data == nullptr) {
    return;
  }
  auto* state = static_cast<CallbackProbeState*>(user_data);
  if (utf8_message != nullptr && utf8_message[0] != '\0') {
    state->kLogCount.fetch_add(1, std::memory_order_relaxed);
  }
}

extern "C" void CaptureDiagnosticsCallback(
    TtCoreDiagnosticSeverity /*severity*/, const char* utf8_message,
    void* user_data) {
  if (user_data == nullptr) {
    return;
  }
  auto* state = static_cast<CallbackProbeState*>(user_data);
  if (utf8_message != nullptr && utf8_message[0] != '\0') {
    state->kDiagnosticsCount.fetch_add(1, std::memory_order_relaxed);
  }
}

extern "C" void CaptureCryptoProgressCallback(const char* utf8_progress_json,
                                              void* user_data) {
  if (user_data == nullptr || utf8_progress_json == nullptr ||
      utf8_progress_json[0] == '\0') {
    return;
  }

  try {
    const json kPayload = json::parse(utf8_progress_json);
    if (!kPayload.is_object()) {
      return;
    }
    auto* state = static_cast<CallbackProbeState*>(user_data);
    state->kCryptoProgressCount.fetch_add(1, std::memory_order_relaxed);
  } catch (...) {
    // Keep callback resilient; test only counts valid JSON payloads.
  }
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
  api.get_build_info =
      RequireSymbol<GetBuildInfoFn>(library, "tracer_core_get_build_info_json");
  api.get_command_contract = RequireSymbol<GetCommandContractFn>(
      library, "tracer_core_get_command_contract_json");
  api.last_error =
      RequireSymbol<LastErrorFn>(library, "tracer_core_last_error");
  api.set_log_callback = RequireSymbol<SetLogCallbackFn>(
      library, "tracer_core_set_log_callback");
  api.set_diagnostics_callback = RequireSymbol<SetDiagnosticsCallbackFn>(
      library, "tracer_core_set_diagnostics_callback");
  api.set_crypto_progress_callback =
      RequireSymbol<SetCryptoProgressCallbackFn>(
          library, "tracer_core_set_crypto_progress_callback");
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
  api.runtime_crypto_encrypt = RequireSymbol<RuntimeCryptoEncryptFn>(
      library, "tracer_core_runtime_crypto_encrypt_json");
  api.runtime_crypto_decrypt = RequireSymbol<RuntimeCryptoDecryptFn>(
      library, "tracer_core_runtime_crypto_decrypt_json");
  api.runtime_crypto_inspect = RequireSymbol<RuntimeCryptoInspectFn>(
      library, "tracer_core_runtime_crypto_inspect_json");
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

  kRequireBool("build_info_json");
  kRequireBool("command_contract_json");
  kRequireBool("runtime_log_callback");
  kRequireBool("runtime_diagnostics_callback");
  kRequireBool("runtime_crypto_progress_callback");
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

  const json kBuildInfo = ParseResponse(api.get_build_info(), "build_info");
  Require(kBuildInfo.value("ok", false),
          "build_info response should return ok=true");
  Require(kBuildInfo.contains("core_version") &&
              kBuildInfo["core_version"].is_string(),
          "build_info response should include string field `core_version`");
  Require(kBuildInfo.contains("abi_version") &&
              kBuildInfo["abi_version"].is_number_integer(),
          "build_info response should include integer field `abi_version`");

  const json kCommandContract =
      ParseResponse(api.get_command_contract(nullptr), "command_contract");
  Require(kCommandContract.value("ok", false),
          "command_contract response should return ok=true");
  Require(kCommandContract.contains("contract_version") &&
              kCommandContract["contract_version"].is_string(),
          "command_contract response should include string `contract_version`");
  Require(kCommandContract.contains("commands") &&
              kCommandContract["commands"].is_array(),
          "command_contract response should include array `commands`");
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
  Require(kRuntimeCheck.contains("error_code") &&
              kRuntimeCheck["error_code"].is_string(),
          "runtime_check_environment response must contain string field "
          "`error_code`");
  Require(kRuntimeCheck.contains("error_category") &&
              kRuntimeCheck["error_category"].is_string(),
          "runtime_check_environment response must contain string field "
          "`error_category`");
  Require(kRuntimeCheck.contains("hints") && kRuntimeCheck["hints"].is_array(),
          "runtime_check_environment response must contain array field "
          "`hints`");
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
  Require(kResolvedContext.contains("error_code") &&
              kResolvedContext["error_code"].is_string(),
          "runtime_resolve_cli_context response must contain string field "
          "`error_code`");
  Require(kResolvedContext.contains("error_category") &&
              kResolvedContext["error_category"].is_string(),
          "runtime_resolve_cli_context response must contain string field "
          "`error_category`");
  Require(kResolvedContext.contains("hints") &&
              kResolvedContext["hints"].is_array(),
          "runtime_resolve_cli_context response must contain array field "
          "`hints`");

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

void RunCallbackBridgeChecks(const CoreApiFns& api,
                             TtCoreRuntimeHandle* runtime,
                             const fs::path& input_root) {
  CallbackProbeState callback_state{};
  api.set_log_callback(&CaptureLogCallback, &callback_state);
  api.set_diagnostics_callback(&CaptureDiagnosticsCallback, &callback_state);
  api.set_crypto_progress_callback(&CaptureCryptoProgressCallback,
                                   &callback_state);

  const std::string kIngestRequest = json{{"input_path", input_root.string()},
                                          {"date_check_mode", "none"},
                                          {"save_processed_output", false}}
                                         .dump();
  RequireOk(api.runtime_ingest(runtime, kIngestRequest.c_str()),
            "callback ingest");

  const fs::path kCryptoInput = input_root / "2026" / "2026-01.txt";
  Require(fs::exists(kCryptoInput),
          "callback crypto input file missing: test/data/2026/2026-01.txt");
  const fs::path kCryptoOutput = input_root / ".." / "output" /
                                 "tracer_core_c_api_stability" / "callback" /
                                 "2026-01.tracer";
  std::error_code io_error;
  fs::create_directories(fs::absolute(kCryptoOutput).parent_path(), io_error);
  Require(!io_error,
          "callback crypto output directory creation failed: " +
              io_error.message());
  const std::string kCryptoEncryptRequest =
      json{{"input_path", fs::absolute(kCryptoInput).string()},
           {"output_path", fs::absolute(kCryptoOutput).string()},
           {"passphrase", "phase6-progress-callback"},
           {"security_level", "interactive"},
           {"date_check_mode", "none"}}
          .dump();
  RequireOk(api.runtime_crypto_encrypt(runtime, kCryptoEncryptRequest.c_str()),
            "callback crypto encrypt");

  api.set_log_callback(nullptr, nullptr);
  api.set_diagnostics_callback(nullptr, nullptr);
  api.set_crypto_progress_callback(nullptr, nullptr);

  const int kLogCount = callback_state.kLogCount.load(std::memory_order_relaxed);
  const int kDiagnosticsCount =
      callback_state.kDiagnosticsCount.load(std::memory_order_relaxed);
  const int kCryptoProgressCount =
      callback_state.kCryptoProgressCount.load(std::memory_order_relaxed);
  Require(kLogCount > 0 || kDiagnosticsCount > 0,
          "runtime callbacks should receive at least one log or diagnostics "
          "message during ingest");
  Require(kCryptoProgressCount > 0,
          "runtime crypto progress callback should receive at least one "
          "valid JSON progress payload during encrypt");
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
