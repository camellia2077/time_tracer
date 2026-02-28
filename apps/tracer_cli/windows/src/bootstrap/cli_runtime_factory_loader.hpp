// bootstrap/cli_runtime_factory_loader.hpp
#pragma once

#include <filesystem>
#include <memory>

#include "api/core_c/tracer_core_c_api.h"

namespace tracer_core::cli::bootstrap::internal {

using PingFn = int (*)(void);
using GetCapabilitiesFn = const char *(*)(void);
using LastErrorFn = const char *(*)(void);
using RuntimeCheckEnvironmentFn = const char *(*)(const char *, int);
using RuntimeResolveCliContextFn = const char *(*)(const char *, const char *,
                                                   const char *, const char *);
using RuntimeCreateFn = TtCoreRuntimeHandle *(*)(const char *, const char *,
                                                 const char *);
using RuntimeDestroyFn = void (*)(TtCoreRuntimeHandle *);
using RuntimeIngestFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeConvertFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeImportFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeValidateStructureFn = const char *(*)(TtCoreRuntimeHandle *,
                                                   const char *);
using RuntimeValidateLogicFn = const char *(*)(TtCoreRuntimeHandle *,
                                               const char *);
using RuntimeQueryFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeReportFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeReportBatchFn = const char *(*)(TtCoreRuntimeHandle *,
                                             const char *);
using RuntimeExportFn = const char *(*)(TtCoreRuntimeHandle *, const char *);
using RuntimeTreeFn = const char *(*)(TtCoreRuntimeHandle *, const char *);

struct CoreApiSymbols {
  PingFn ping = nullptr;
  GetCapabilitiesFn get_capabilities = nullptr;
  LastErrorFn last_error = nullptr;
  RuntimeCheckEnvironmentFn runtime_check_environment_json = nullptr;
  RuntimeResolveCliContextFn runtime_resolve_cli_context_json = nullptr;
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

class CoreLibrary final {
public:
  ~CoreLibrary();
  CoreLibrary(const CoreLibrary &) = delete;
  auto operator=(const CoreLibrary &) -> CoreLibrary & = delete;

  [[nodiscard]] static auto LoadFromExeDir(const std::filesystem::path &exe_dir)
      -> std::shared_ptr<CoreLibrary>;
  [[nodiscard]] auto symbols() const -> const CoreApiSymbols &;

private:
  struct Impl;
  explicit CoreLibrary(std::unique_ptr<Impl> impl);

  std::unique_ptr<Impl> impl_;
};

} // namespace tracer_core::cli::bootstrap::internal
