// tracer_windows/src/bootstrap/cli_runtime_factory_loader.cpp
#include "bootstrap/cli_runtime_factory_loader.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "shared/types/exceptions.hpp"

namespace time_tracer::cli::bootstrap::internal {

namespace fs = std::filesystem;

namespace {

using time_tracer::common::DllCompatibilityError;

constexpr std::string_view kCoreLibraryName = "time_tracer_core.dll";

#if defined(_WIN32)
using LibraryHandle = HMODULE;
auto OpenLibrary(const fs::path &library_path) -> LibraryHandle {
  return LoadLibraryW(library_path.wstring().c_str());
}
auto LookupSymbol(LibraryHandle handle, const char *name) -> void * {
  return reinterpret_cast<void *>(GetProcAddress(handle, name));
}
void CloseLibrary(LibraryHandle handle) {
  if (handle != nullptr) {
    FreeLibrary(handle);
  }
}
auto LastLoaderError() -> std::string {
  return "Win32Error=" +
         std::to_string(static_cast<unsigned long>(GetLastError()));
}
#else
using LibraryHandle = void *;
auto OpenLibrary(const fs::path &library_path) -> LibraryHandle {
  return dlopen(library_path.string().c_str(), RTLD_NOW);
}
auto LookupSymbol(LibraryHandle handle, const char *name) -> void * {
  return dlsym(handle, name);
}
void CloseLibrary(LibraryHandle handle) {
  if (handle != nullptr) {
    dlclose(handle);
  }
}
auto LastLoaderError() -> std::string {
  const char *error = dlerror();
  return error != nullptr ? std::string(error) : std::string("unknown");
}
#endif

[[nodiscard]] auto CoreLibraryPath(const fs::path &base_dir) -> fs::path {
  return base_dir / kCoreLibraryName;
}

template <typename Fn>
auto RequireSymbol(LibraryHandle handle, const char *symbol_name) -> Fn {
  void *symbol = LookupSymbol(handle, symbol_name);
  if (symbol == nullptr) {
    throw DllCompatibilityError("Missing required symbol: " +
                                std::string(symbol_name));
  }
  return reinterpret_cast<Fn>(symbol);
}

[[nodiscard]] auto BindCoreApiSymbols(LibraryHandle handle) -> CoreApiSymbols {
  CoreApiSymbols symbols{};
  symbols.ping = RequireSymbol<PingFn>(handle, "tracer_core_ping");
  symbols.get_capabilities = reinterpret_cast<GetCapabilitiesFn>(
      LookupSymbol(handle, "tracer_core_get_capabilities_json"));
  symbols.last_error =
      RequireSymbol<LastErrorFn>(handle, "tracer_core_last_error");
  symbols.runtime_check_environment_json =
      RequireSymbol<RuntimeCheckEnvironmentFn>(
          handle, "tracer_core_runtime_check_environment_json");
  symbols.runtime_resolve_cli_context_json =
      RequireSymbol<RuntimeResolveCliContextFn>(
          handle, "tracer_core_runtime_resolve_cli_context_json");
  symbols.runtime_create =
      RequireSymbol<RuntimeCreateFn>(handle, "tracer_core_runtime_create");
  symbols.runtime_destroy =
      RequireSymbol<RuntimeDestroyFn>(handle, "tracer_core_runtime_destroy");
  symbols.runtime_ingest =
      RequireSymbol<RuntimeIngestFn>(handle, "tracer_core_runtime_ingest_json");
  symbols.runtime_convert = RequireSymbol<RuntimeConvertFn>(
      handle, "tracer_core_runtime_convert_json");
  symbols.runtime_import =
      RequireSymbol<RuntimeImportFn>(handle, "tracer_core_runtime_import_json");
  symbols.runtime_validate_structure =
      RequireSymbol<RuntimeValidateStructureFn>(
          handle, "tracer_core_runtime_validate_structure_json");
  symbols.runtime_validate_logic = RequireSymbol<RuntimeValidateLogicFn>(
      handle, "tracer_core_runtime_validate_logic_json");
  symbols.runtime_query =
      RequireSymbol<RuntimeQueryFn>(handle, "tracer_core_runtime_query_json");
  symbols.runtime_report =
      RequireSymbol<RuntimeReportFn>(handle, "tracer_core_runtime_report_json");
  symbols.runtime_report_batch = RequireSymbol<RuntimeReportBatchFn>(
      handle, "tracer_core_runtime_report_batch_json");
  symbols.runtime_export =
      RequireSymbol<RuntimeExportFn>(handle, "tracer_core_runtime_export_json");
  symbols.runtime_tree =
      RequireSymbol<RuntimeTreeFn>(handle, "tracer_core_runtime_tree_json");
  return symbols;
}

} // namespace

struct CoreLibrary::Impl {
  explicit Impl(LibraryHandle in_handle, CoreApiSymbols in_symbols)
      : handle(in_handle), symbols(std::move(in_symbols)) {}

  LibraryHandle handle = nullptr;
  CoreApiSymbols symbols{};
};

CoreLibrary::CoreLibrary(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

CoreLibrary::~CoreLibrary() {
  if (impl_ && impl_->handle != nullptr) {
    CloseLibrary(impl_->handle);
    impl_->handle = nullptr;
  }
}

auto CoreLibrary::LoadFromExeDir(const fs::path &exe_dir)
    -> std::shared_ptr<CoreLibrary> {
  const fs::path library_path = CoreLibraryPath(exe_dir);
  if (!fs::exists(library_path)) {
    throw DllCompatibilityError("Core runtime library not found: " +
                                library_path.string());
  }
  LibraryHandle handle = OpenLibrary(library_path);
  if (handle == nullptr) {
    throw DllCompatibilityError("Failed to load " + library_path.string() +
                                ": " + LastLoaderError());
  }

  CoreApiSymbols symbols{};
  try {
    symbols = BindCoreApiSymbols(handle);
  } catch (...) {
    CloseLibrary(handle);
    throw;
  }

  return std::shared_ptr<CoreLibrary>(
      new CoreLibrary(std::make_unique<Impl>(handle, std::move(symbols))));
}

auto CoreLibrary::symbols() const -> const CoreApiSymbols & {
  return impl_->symbols;
}

} // namespace time_tracer::cli::bootstrap::internal
