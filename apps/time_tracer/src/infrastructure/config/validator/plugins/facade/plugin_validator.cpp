// infrastructure/config/validator/plugins/facade/plugin_validator.cpp
#include "infrastructure/config/validator/plugins/facade/plugin_validator.hpp"

#include <optional>
#include <string>
#include <string_view>

#include "domain/ports/diagnostics.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace fs = std::filesystem;

namespace {
#ifdef _WIN32
constexpr std::string_view kDynamicLibraryExtension = ".dll";
// NOLINTNEXTLINE(readability-use-concise-preprocessor-directives)
#elif defined(__APPLE__)
constexpr std::string_view kDynamicLibraryExtension = ".dylib";
#else
constexpr std::string_view kDynamicLibraryExtension = ".so";
#endif

constexpr const char* kAbiInfoSymbol = "tt_getFormatterAbiInfo";

class DynamicLibrary {
 public:
  explicit DynamicLibrary(const fs::path& library_path) {
#ifdef _WIN32
    handle_ = LoadLibraryA(library_path.string().c_str());
#else
    handle_ = dlopen(library_path.string().c_str(), RTLD_LAZY);
#endif
  }

  ~DynamicLibrary() {
#ifdef _WIN32
    if (handle_ != nullptr) {
      FreeLibrary(handle_);
    }
#else
    if (handle_ != nullptr) {
      dlclose(handle_);
    }
#endif
  }

  DynamicLibrary(const DynamicLibrary&) = delete;
  auto operator=(const DynamicLibrary&) -> DynamicLibrary& = delete;

  [[nodiscard]] auto IsOpen() const -> bool { return handle_ != nullptr; }

  [[nodiscard]] auto GetSymbol(const char* symbol_name) const -> void* {
#ifdef _WIN32
    if (handle_ == nullptr) {
      return nullptr;
    }
    return reinterpret_cast<void*>(GetProcAddress(handle_, symbol_name));
#else
    if (handle_ == nullptr) {
      return nullptr;
    }
    dlerror();
    return dlsym(handle_, symbol_name);
#endif
  }

 private:
#ifdef _WIN32
  HMODULE handle_ = nullptr;
#else
  void* handle_ = nullptr;
#endif
};

[[nodiscard]] auto GetDynamicLibraryErrorMessage() -> std::string {
#ifdef _WIN32
  return "Win32 error code: " +
         std::to_string(static_cast<unsigned long>(GetLastError()));
#else
  const char* error = dlerror();
  if (error == nullptr) {
    return "unknown dynamic loader error";
  }
  return error;
#endif
}

[[nodiscard]] auto ResolvePluginBinaryPath(const fs::path& plugins_path,
                                           const std::string& plugin_name)
    -> std::optional<fs::path> {
  const std::string kExtension(kDynamicLibraryExtension);

  const fs::path kDllPathWithPrefix =
      plugins_path / ("lib" + plugin_name + kExtension);
  if (FileSystemHelper::Exists(kDllPathWithPrefix)) {
    return kDllPathWithPrefix;
  }

  const fs::path kDllPathWithoutPrefix =
      plugins_path / (plugin_name + kExtension);
  if (FileSystemHelper::Exists(kDllPathWithoutPrefix)) {
    return kDllPathWithoutPrefix;
  }

  return std::nullopt;
}

[[nodiscard]] auto GetExpectedAbiVersion(const PluginValidationOptions& options)
    -> uint32_t {
  if (options.expected_formatter_abi_version != 0U) {
    return options.expected_formatter_abi_version;
  }
  return TT_FORMATTER_ABI_VERSION_CURRENT;
}

auto ValidateFormatterAbi(const fs::path& plugin_path,
                          uint32_t expected_abi_version) -> bool {
  DynamicLibrary library(plugin_path);
  if (!library.IsOpen()) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Failed to load plugin DLL at '" +
        plugin_path.string() + "': " + GetDynamicLibraryErrorMessage() + ".");
    return false;
  }

  auto get_abi_info = reinterpret_cast<TtGetFormatterAbiInfoFuncV2>(
      library.GetSymbol(kAbiInfoSymbol));
  if (get_abi_info == nullptr) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Required symbol '" + std::string(kAbiInfoSymbol) +
        "' not found in '" + plugin_path.string() + "'.");
    return false;
  }

  TtFormatterAbiInfo abi_info{};
  abi_info.structSize = static_cast<uint32_t>(sizeof(TtFormatterAbiInfo));
  const auto kStatusCode =
      static_cast<TtFormatterStatusCode>(get_abi_info(&abi_info));
  if (kStatusCode != TT_FORMATTER_STATUS_OK) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: '" + std::string(kAbiInfoSymbol) +
        "' failed for '" + plugin_path.string() + "' with status code " +
        std::to_string(static_cast<int32_t>(kStatusCode)) + ".");
    return false;
  }

  if (abi_info.abiVersion != expected_abi_version) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: ABI mismatch for '" + plugin_path.string() +
        "'. Expected " + std::to_string(expected_abi_version) + ", got " +
        std::to_string(abi_info.abiVersion) + ".");
    return false;
  }

  return true;
}
}  // namespace

auto PluginValidator::Validate(const fs::path& plugins_path,
                               const std::vector<std::string>& expected_plugins,
                               const PluginValidationOptions& options) -> bool {
  if (!FileSystemHelper::Exists(plugins_path) ||
      !FileSystemHelper::IsDirectory(plugins_path)) {
    time_tracer::domain::ports::EmitError(
        "[Validator] Error: Plugins directory not found at '" +
        plugins_path.string() + "'.");
    return expected_plugins.empty();
  }

  bool all_valid = true;
  const uint32_t kExpectedAbiVersion = GetExpectedAbiVersion(options);

  for (const auto& plugin_name : expected_plugins) {
    const auto kPluginPath = ResolvePluginBinaryPath(plugins_path, plugin_name);
    if (!kPluginPath.has_value()) {
      time_tracer::domain::ports::EmitError(
          "[Validator] Error: Required plugin '" + plugin_name +
          std::string(kDynamicLibraryExtension) + "' not found in directory '" +
          plugins_path.string() + "'.");
      all_valid = false;
      continue;
    }

    if (options.require_formatter_abi &&
        !ValidateFormatterAbi(*kPluginPath, kExpectedAbiVersion)) {
      all_valid = false;
    }
  }

  return all_valid;
}
