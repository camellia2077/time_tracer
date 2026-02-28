// api/core_c/tracer_core_c_api_smoke_tests.cpp
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "api/core_c/tracer_core_c_api.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

using GetVersionFn = const char* (*)();
using PingFn = int (*)();
using GetCapabilitiesFn = const char* (*)();
using LastErrorFn = const char* (*)();
using RuntimeCreateFn = TtCoreRuntimeHandle* (*)(const char*, const char*,
                                                 const char*);
using RuntimeDestroyFn = void (*)(TtCoreRuntimeHandle*);
using RuntimeIngestFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeQueryFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeReportFn = const char* (*)(TtCoreRuntimeHandle*, const char*);

#ifdef _WIN32
using LibHandle = HMODULE;

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

constexpr const char* kLibraryName = "tracer_core.dll";
#else
using LibHandle = void*;

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

constexpr const char* kLibraryName = "libtracer_core.so";
#endif

auto FindRepoRoot() -> std::filesystem::path {
  namespace fs = std::filesystem;
  fs::path current = fs::current_path();
  while (!current.empty()) {
    const fs::path kConfigProbe = current / "apps" / "tracer_core" / "config" /
                                  "converter" /
                                  "interval_processor_config.toml";
    if (fs::exists(kConfigProbe)) {
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

auto IsOkResponse(const char* response_json, std::string_view context,
                  std::string* error_out) -> bool {
  if (response_json == nullptr || response_json[0] == '\0') {
    *error_out = std::string(context) + ": empty response";
    return false;
  }
  nlohmann::json payload;
  try {
    payload = nlohmann::json::parse(response_json);
  } catch (const std::exception& error) {
    *error_out =
        std::string(context) + ": invalid json response: " + error.what();
    return false;
  }
  const auto kOkIt = payload.find("ok");
  if (kOkIt == payload.end() || !kOkIt->is_boolean()) {
    *error_out = std::string(context) + ": response has no boolean `ok`";
    return false;
  }
  if (!kOkIt->get<bool>()) {
    std::string error_message;
    const auto kErrorIt = payload.find("error_message");
    if (kErrorIt != payload.end() && kErrorIt->is_string()) {
      error_message = kErrorIt->get<std::string>();
    }
    *error_out = std::string(context) + ": ok=false, error=" + error_message;
    return false;
  }
  return true;
}

}  // namespace

auto main() -> int {
  LibHandle library = nullptr;
  try {
    library = OpenLibrary(kLibraryName);
    if (library == nullptr) {
      std::cerr << "[FAIL] unable to load " << kLibraryName << ": "
                << LastDynamicError() << '\n';
      return 1;
    }

    const auto kGetVersion = reinterpret_cast<GetVersionFn>(
        LookupSymbol(library, "tracer_core_get_version"));
    const auto kPing =
        reinterpret_cast<PingFn>(LookupSymbol(library, "tracer_core_ping"));
    const auto kGetCapabilities = reinterpret_cast<GetCapabilitiesFn>(
        LookupSymbol(library, "tracer_core_get_capabilities_json"));
    const auto kLastError = reinterpret_cast<LastErrorFn>(
        LookupSymbol(library, "tracer_core_last_error"));
    const auto kRuntimeCreate = reinterpret_cast<RuntimeCreateFn>(
        LookupSymbol(library, "tracer_core_runtime_create"));
    const auto kRuntimeDestroy = reinterpret_cast<RuntimeDestroyFn>(
        LookupSymbol(library, "tracer_core_runtime_destroy"));
    const auto kRuntimeIngest = reinterpret_cast<RuntimeIngestFn>(
        LookupSymbol(library, "tracer_core_runtime_ingest_json"));
    const auto kRuntimeQuery = reinterpret_cast<RuntimeQueryFn>(
        LookupSymbol(library, "tracer_core_runtime_query_json"));
    const auto kRuntimeReport = reinterpret_cast<RuntimeReportFn>(
        LookupSymbol(library, "tracer_core_runtime_report_json"));

    if (kGetVersion == nullptr || kPing == nullptr ||
        kGetCapabilities == nullptr || kLastError == nullptr ||
        kRuntimeCreate == nullptr || kRuntimeDestroy == nullptr ||
        kRuntimeIngest == nullptr || kRuntimeQuery == nullptr ||
        kRuntimeReport == nullptr) {
      std::cerr << "[FAIL] missing required exported symbols in "
                << kLibraryName << '\n';
      CloseLibrary(library);
      return 1;
    }

    const int kPingResult = kPing();
    if (kPingResult != TT_CORE_STATUS_OK) {
      std::cerr << "[FAIL] tracer_core_ping returned " << kPingResult
                << ", error=" << kLastError() << '\n';
      CloseLibrary(library);
      return 1;
    }

    const char* version = kGetVersion();
    if (version == nullptr || std::string(version).empty()) {
      std::cerr << "[FAIL] tracer_core_get_version returned an empty string.\n";
      CloseLibrary(library);
      return 1;
    }
    {
      nlohmann::json capability_payload;
      try {
        capability_payload = nlohmann::json::parse(kGetCapabilities());
      } catch (const std::exception& error) {
        std::cerr << "[FAIL] tracer_core_get_capabilities_json invalid json: "
                  << error.what() << '\n';
        CloseLibrary(library);
        return 1;
      }
      const auto kFeaturesIt = capability_payload.find("features");
      if (kFeaturesIt == capability_payload.end() ||
          !kFeaturesIt->is_object()) {
        std::cerr << "[FAIL] tracer_core_get_capabilities_json missing "
                     "`features` object.\n";
        CloseLibrary(library);
        return 1;
      }
    }

    const std::filesystem::path kRepoRoot = FindRepoRoot();
    if (kRepoRoot.empty()) {
      std::cerr
          << "[FAIL] unable to locate repo root from current directory.\n";
      CloseLibrary(library);
      return 1;
    }

    const std::filesystem::path kConverterConfig =
        kRepoRoot / "apps" / "tracer_core" / "config" / "converter" /
        "interval_processor_config.toml";
    const std::filesystem::path kInputRoot = kRepoRoot / "test" / "data";
    if (!std::filesystem::exists(kInputRoot)) {
      std::cerr << "[FAIL] missing test input root: " << kInputRoot << '\n';
      CloseLibrary(library);
      return 1;
    }

    const std::filesystem::path kTempRoot =
        kRepoRoot / "test" / "output" / "tracer_core_c_api_smoke";
    const std::filesystem::path kOutputRoot = kTempRoot / "runtime_output";
    const std::filesystem::path kDbPath =
        kOutputRoot / "db" / "time_data.sqlite3";

    std::error_code io_error;
    std::filesystem::remove_all(kTempRoot, io_error);
    std::filesystem::create_directories(kOutputRoot, io_error);
    if (io_error) {
      std::cerr << "[FAIL] unable to create temp output folder: "
                << io_error.message() << '\n';
      CloseLibrary(library);
      return 1;
    }

    TtCoreRuntimeHandle* runtime_handle =
        kRuntimeCreate(kDbPath.string().c_str(), kOutputRoot.string().c_str(),
                       kConverterConfig.string().c_str());
    if (runtime_handle == nullptr) {
      std::cerr << "[FAIL] tracer_core_runtime_create failed: " << kLastError()
                << '\n';
      CloseLibrary(library);
      return 1;
    }

    std::string response_error;
    const std::string kIngestRequest = nlohmann::json{
        {"input_path", kInputRoot.string()},
        {"date_check_mode", "none"},
        {"save_processed_output", false}}.dump();
    if (!IsOkResponse(kRuntimeIngest(runtime_handle, kIngestRequest.c_str()),
                      "ingest", &response_error)) {
      std::cerr << "[FAIL] " << response_error << '\n';
      kRuntimeDestroy(runtime_handle);
      CloseLibrary(library);
      return 1;
    }

    const std::string kQueryRequest =
        nlohmann::json{{"action", "years"}}.dump();
    if (!IsOkResponse(kRuntimeQuery(runtime_handle, kQueryRequest.c_str()),
                      "query", &response_error)) {
      std::cerr << "[FAIL] " << response_error << '\n';
      kRuntimeDestroy(runtime_handle);
      CloseLibrary(library);
      return 1;
    }

    const std::string kReportRequest =
        nlohmann::json{
            {"type", "day"}, {"argument", "2021-01-01"}, {"format", "markdown"}}
            .dump();
    if (!IsOkResponse(kRuntimeReport(runtime_handle, kReportRequest.c_str()),
                      "report", &response_error)) {
      std::cerr << "[FAIL] " << response_error << '\n';
      kRuntimeDestroy(runtime_handle);
      CloseLibrary(library);
      return 1;
    }

    kRuntimeDestroy(runtime_handle);

    std::cout << "[PASS] tracer_core_c_api_smoke_tests version=" << version
              << '\n';
    CloseLibrary(library);
    return 0;
  } catch (const std::exception& error) {
    if (library != nullptr) {
      CloseLibrary(library);
    }
    std::cerr << "[FAIL] tracer_core_c_api_smoke_tests: " << error.what()
              << '\n';
    return 1;
  } catch (...) {
    if (library != nullptr) {
      CloseLibrary(library);
    }
    std::cerr << "[FAIL] tracer_core_c_api_smoke_tests: unknown error\n";
    return 1;
  }
}
