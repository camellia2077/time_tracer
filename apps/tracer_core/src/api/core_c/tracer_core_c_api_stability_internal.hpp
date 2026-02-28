// api/core_c/tracer_core_c_api_stability_internal.hpp
#ifndef API_CORE_C_TRACER_CORE_C_API_STABILITY_INTERNAL_HPP_
#define API_CORE_C_TRACER_CORE_C_API_STABILITY_INTERNAL_HPP_

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

#include "api/core_c/tracer_core_c_api.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace tracer_core_c_api_stability_internal {

namespace fs = std::filesystem;
using nlohmann::json;

using GetVersionFn = const char* (*)();
using PingFn = int (*)();
using GetCapabilitiesFn = const char* (*)();
using LastErrorFn = const char* (*)();
using RuntimeCheckEnvironmentFn = const char* (*)(const char*, int);
using RuntimeResolveCliContextFn = const char* (*)(const char*, const char*,
                                                   const char*, const char*);
using RuntimeCreateFn = TtCoreRuntimeHandle* (*)(const char*, const char*,
                                                 const char*);
using RuntimeDestroyFn = void (*)(TtCoreRuntimeHandle*);
using RuntimeIngestFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeConvertFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeImportFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeValidateStructureFn = const char* (*)(TtCoreRuntimeHandle*,
                                                   const char*);
using RuntimeValidateLogicFn = const char* (*)(TtCoreRuntimeHandle*,
                                               const char*);
using RuntimeQueryFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeReportFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeReportBatchFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeExportFn = const char* (*)(TtCoreRuntimeHandle*, const char*);
using RuntimeTreeFn = const char* (*)(TtCoreRuntimeHandle*, const char*);

struct CoreApiFns {
  GetVersionFn get_version = nullptr;
  PingFn ping = nullptr;
  GetCapabilitiesFn get_capabilities = nullptr;
  LastErrorFn last_error = nullptr;
  RuntimeCheckEnvironmentFn runtime_check_environment = nullptr;
  RuntimeResolveCliContextFn runtime_resolve_cli_context = nullptr;
  RuntimeCreateFn runtime_create = nullptr;
  RuntimeDestroyFn runtime_destroy = nullptr;
  RuntimeIngestFn runtime_ingest = nullptr;
  RuntimeConvertFn runtime_convert = nullptr;
  RuntimeImportFn runtime_import = nullptr;
  RuntimeValidateStructureFn runtime_validate_structure = nullptr;
  RuntimeValidateLogicFn runtime_validate_logic = nullptr;
  RuntimeQueryFn runtime_query = nullptr;
  RuntimeReportFn runtime_report = nullptr;
  RuntimeReportBatchFn runtime_report_batch = nullptr;
  RuntimeExportFn runtime_export = nullptr;
  RuntimeTreeFn runtime_tree = nullptr;
};

#ifdef _WIN32
using LibHandle = HMODULE;
inline constexpr const char* kLibraryName = "tracer_core.dll";
#else
using LibHandle = void*;
inline constexpr const char* kLibraryName = "libtracer_core.so";
#endif

class RuntimeGuard {
 public:
  RuntimeGuard(RuntimeDestroyFn destroy_fn, TtCoreRuntimeHandle* handle);
  ~RuntimeGuard();

  RuntimeGuard(const RuntimeGuard&) = delete;
  auto operator=(const RuntimeGuard&) -> RuntimeGuard& = delete;

  [[nodiscard]] auto Get() const -> TtCoreRuntimeHandle*;

 private:
  RuntimeDestroyFn destroy_fn_ = nullptr;
  TtCoreRuntimeHandle* handle_ = nullptr;
};

struct RuntimePathBundle {
  const fs::path& db_path;
  const fs::path& temp_root;
  const fs::path& converter_config;
};

auto OpenLibrary(const char* library_name) -> LibHandle;
auto LookupSymbol(LibHandle handle, const char* symbol_name) -> void*;
void CloseLibrary(LibHandle handle);
auto LastDynamicError() -> std::string;

void Require(bool condition, const std::string& message);
auto LoadApi(LibHandle library) -> CoreApiFns;
auto FindRepoRoot() -> fs::path;
auto ParseResponse(const char* response_json, std::string_view context) -> json;
void RequireOk(const char* response_json, std::string_view context);
void RequireNotOk(const char* response_json, std::string_view context);
void RunCapabilitiesChecks(const CoreApiFns& api);
void RunRuntimeBootstrapChecks(const CoreApiFns& api,
                               const fs::path& executable_path);
auto CreateRuntime(const CoreApiFns& api, const fs::path& db_path,
                   const fs::path& output_root,
                   const fs::path& converter_config) -> RuntimeGuard;
void RunConcurrentChecks(const CoreApiFns& api, const RuntimePathBundle& paths);
void RunCreateDestroyChurn(const CoreApiFns& api,
                           const RuntimePathBundle& paths);

void RunQueryChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime);
void RunErrorPathChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                        const fs::path& converter_config);

}  // namespace tracer_core_c_api_stability_internal

#endif  // API_CORE_C_TRACER_CORE_C_API_STABILITY_INTERNAL_HPP_
