#include "api/core_c/time_tracer_core_c_api.h"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "api/core_c/time_tracer_core_c_api_internal.hpp"
#include "infrastructure/config/config_loader.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "shared/types/version.hpp"

using time_tracer::core::c_api::internal::BuildCapabilitiesResponseJson;
using time_tracer::core::c_api::internal::ClearLastError;
using time_tracer::core::c_api::internal::SetLastError;

namespace {

namespace fs = std::filesystem;
using nlohmann::json;

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";
constexpr std::string_view kCoreLibraryName = "time_tracer_core.dll";
constexpr std::string_view kReportsSharedLibraryName = "libreports_shared.dll";
constexpr std::string_view kSqliteLibraryName = "libsqlite3-0.dll";
constexpr std::string_view kTomlLibraryName = "libtomlplusplus-3.dll";

struct ResolvedCliContext {
  AppConfig app_config;
  fs::path output_root;
  fs::path db_path;
  fs::path export_root;
  fs::path runtime_output_root;
};

[[nodiscard]] auto IsNonEmptyCString(const char* value) -> bool {
  return value != nullptr && value[0] != '\0';
}

[[nodiscard]] auto ToDateCheckModeString(DateCheckMode mode)
    -> std::string_view {
  switch (mode) {
    case DateCheckMode::kNone:
      return "none";
    case DateCheckMode::kContinuity:
      return "continuity";
    case DateCheckMode::kFull:
      return "full";
  }
  return "none";
}

[[nodiscard]] auto ToDateCheckModeJson(const std::optional<DateCheckMode>& mode)
    -> json {
  if (!mode.has_value()) {
    return nullptr;
  }
  return std::string(ToDateCheckModeString(*mode));
}

[[nodiscard]] auto ToStringJson(const std::optional<std::string>& value)
    -> json {
  if (!value.has_value()) {
    return nullptr;
  }
  return *value;
}

[[nodiscard]] auto BuildCliConfigJson(const AppConfig& app_config) -> json {
  return json{
      {"default_save_processed_output",
       app_config.default_save_processed_output},
      {"default_date_check_mode",
       std::string(ToDateCheckModeString(app_config.default_date_check_mode))},
      {"defaults",
       {{"default_format", ToStringJson(app_config.defaults.default_format)}}},
      {"command_defaults",
       {{"export_format",
         ToStringJson(app_config.command_defaults.export_format)},
        {"query_format",
         ToStringJson(app_config.command_defaults.query_format)},
        {"convert_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.convert_date_check_mode)},
        {"convert_save_processed_output",
         app_config.command_defaults.convert_save_processed_output.has_value()
             ? json(*app_config.command_defaults.convert_save_processed_output)
             : json(nullptr)},
        {"convert_validate_logic",
         app_config.command_defaults.convert_validate_logic.has_value()
             ? json(*app_config.command_defaults.convert_validate_logic)
             : json(nullptr)},
        {"convert_validate_structure",
         app_config.command_defaults.convert_validate_structure.has_value()
             ? json(*app_config.command_defaults.convert_validate_structure)
             : json(nullptr)},
        {"ingest_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.ingest_date_check_mode)},
        {"ingest_save_processed_output",
         app_config.command_defaults.ingest_save_processed_output.has_value()
             ? json(*app_config.command_defaults.ingest_save_processed_output)
             : json(nullptr)},
        {"validate_logic_date_check_mode",
         ToDateCheckModeJson(
             app_config.command_defaults.validate_logic_date_check_mode)}}}};
}

[[nodiscard]] auto ResolveCliContext(const char* executable_path,
                                     const char* db_override,
                                     const char* output_override,
                                     const char* command_name)
    -> ResolvedCliContext {
  if (!IsNonEmptyCString(executable_path)) {
    throw std::invalid_argument("executable_path must not be empty.");
  }

  ConfigLoader config_loader(executable_path);
  AppConfig app_config = config_loader.LoadConfiguration();

  const fs::path kExecutable = fs::absolute(fs::path(executable_path));
  fs::path output_root = fs::absolute(kExecutable.parent_path() / "output");
  if (app_config.defaults.output_root.has_value()) {
    output_root = fs::absolute(*app_config.defaults.output_root);
  }

  fs::path export_root;
  if (IsNonEmptyCString(output_override)) {
    export_root = fs::absolute(fs::path(output_override));
  } else if (app_config.kExportPath.has_value()) {
    export_root = fs::absolute(*app_config.kExportPath);
  }

  fs::path db_path = output_root / "db" / kDatabaseFilename;
  if (app_config.defaults.kDbPath.has_value()) {
    db_path = fs::absolute(*app_config.defaults.kDbPath);
  }
  if (IsNonEmptyCString(db_override)) {
    db_path = fs::absolute(fs::path(db_override));
  }

  const std::string kResolvedCommand = IsNonEmptyCString(command_name)
                                           ? std::string(command_name)
                                           : std::string();
  const fs::path kRuntimeOutputRoot =
      (kResolvedCommand == "export" && !export_root.empty()) ? export_root
                                                             : output_root;

  return ResolvedCliContext{
      .app_config = std::move(app_config),
      .output_root = std::move(output_root),
      .db_path = std::move(db_path),
      .export_root = std::move(export_root),
      .runtime_output_root = kRuntimeOutputRoot,
  };
}

[[nodiscard]] auto BuildFailureJsonResponse(
    std::string_view error_message,
    const std::vector<std::string>& messages = {}) -> const char* {
  const std::string kNormalizedError = error_message.empty()
                                           ? std::string("Unknown error.")
                                           : std::string(error_message);
  SetLastError(kNormalizedError.c_str());

  json payload = {{"ok", false}, {"error_message", kNormalizedError}};
  if (!messages.empty()) {
    payload["messages"] = messages;
  }
  time_tracer::core::c_api::internal::g_last_response = payload.dump();
  return time_tracer::core::c_api::internal::g_last_response.c_str();
}

[[nodiscard]] auto BuildSuccessJsonResponse(const json& payload) -> const
    char* {
  time_tracer::core::c_api::internal::g_last_response = payload.dump();
  return time_tracer::core::c_api::internal::g_last_response.c_str();
}

}  // namespace

extern "C" TT_CORE_API auto tracer_core_get_version(void) -> const char* {
  try {
    ClearLastError();
    return AppInfo::kVersion.data();
  } catch (...) {
    SetLastError("tracer_core_get_version failed unexpectedly.");
    return "";
  }
}

extern "C" TT_CORE_API auto tracer_core_ping(void) -> int {
  try {
    ClearLastError();
    return TT_CORE_STATUS_OK;
  } catch (...) {
    SetLastError("tracer_core_ping failed unexpectedly.");
    return TT_CORE_STATUS_ERROR;
  }
}

extern "C" TT_CORE_API auto tracer_core_get_capabilities_json(void) -> const
    char* {
  try {
    ClearLastError();
    return BuildCapabilitiesResponseJson();
  } catch (const std::exception& error) {
    SetLastError(error.what());
    return "{}";
  } catch (...) {
    SetLastError("tracer_core_get_capabilities_json failed unexpectedly.");
    return "{}";
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_check_environment_json(
    const char* executable_path, int is_help_mode) -> const char* {
  try {
    ClearLastError();
    const ResolvedCliContext kContext =
        ResolveCliContext(executable_path, nullptr, nullptr, nullptr);
    if (is_help_mode != 0) {
      return BuildSuccessJsonResponse(json{
          {"ok", true}, {"error_message", ""}, {"messages", json::array()}});
    }

    const fs::path kBinDir = kContext.app_config.exe_dir_path;
    std::vector<std::string> errors;
    const std::vector<fs::path> kRequiredFiles = {
        kBinDir / kCoreLibraryName,         kBinDir / kReportsSharedLibraryName,
        kBinDir / kSqliteLibraryName,       kBinDir / kTomlLibraryName,
        kBinDir / "config" / "config.toml",
    };

    for (const auto& path : kRequiredFiles) {
      if (!fs::exists(path)) {
        errors.emplace_back("[runtime-check] missing required runtime file: " +
                            path.string());
      }
    }

    if (tracer_core_ping() != TT_CORE_STATUS_OK) {
      const char* error = tracer_core_last_error();
      errors.emplace_back(std::string("[runtime-check] core ping failed: ") +
                          (error != nullptr ? error : "unknown"));
    }

    const char* capabilities_json = tracer_core_get_capabilities_json();
    if (capabilities_json == nullptr || capabilities_json[0] == '\0') {
      errors.emplace_back(
          "[runtime-check] tracer_core_get_capabilities_json returned empty "
          "payload.");
    } else {
      try {
        const json kCapabilities = json::parse(capabilities_json);
        const auto kFeaturesIt = kCapabilities.find("features");
        if (kFeaturesIt == kCapabilities.end() || !kFeaturesIt->is_object()) {
          errors.emplace_back(
              "[runtime-check] capabilities payload missing `features` "
              "object.");
        }
      } catch (const std::exception& error) {
        errors.emplace_back(std::string("[runtime-check] invalid capabilities "
                                        "payload: ") +
                            error.what());
      }
    }

    if (!errors.empty()) {
      return BuildFailureJsonResponse(errors.front(), errors);
    }
    return BuildSuccessJsonResponse(
        json{{"ok", true}, {"error_message", ""}, {"messages", json::array()}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what());
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_runtime_check_environment_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_runtime_resolve_cli_context_json(
    const char* executable_path, const char* db_override,
    const char* output_override, const char* command_name) -> const char* {
  try {
    ClearLastError();
    const ResolvedCliContext kContext = ResolveCliContext(
        executable_path, db_override, output_override, command_name);
    return BuildSuccessJsonResponse(
        json{{"ok", true},
             {"error_message", ""},
             {"paths",
              {{"exe_dir", kContext.app_config.exe_dir_path.string()},
               {"db_path", kContext.db_path.string()},
               {"output_root", kContext.output_root.string()},
               {"export_root", kContext.export_root.string()},
               {"runtime_output_root", kContext.runtime_output_root.string()},
               {"converter_config_toml_path",
                kContext.app_config.pipeline.interval_processor_config_path
                    .string()}}},
             {"cli_config", BuildCliConfigJson(kContext.app_config)}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what());
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_runtime_resolve_cli_context_json failed unexpectedly.");
  }
}

extern "C" TT_CORE_API auto tracer_core_last_error(void) -> const char* {
  if (time_tracer::core::c_api::internal::g_last_error.empty()) {
    return "";
  }
  return time_tracer::core::c_api::internal::g_last_error.c_str();
}

// ABI compatibility: parameter order is part of exported C contract.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
extern "C" TT_CORE_API auto tracer_core_runtime_create(
    const char* db_path, const char* output_root,
    const char* converter_config_toml_path) -> TtCoreRuntimeHandle* {
  try {
    ClearLastError();
    if (output_root == nullptr || output_root[0] == '\0') {
      throw std::invalid_argument("output_root must not be empty.");
    }
    if (converter_config_toml_path == nullptr ||
        converter_config_toml_path[0] == '\0') {
      throw std::invalid_argument(
          "converter_config_toml_path must not be empty.");
    }

    infrastructure::bootstrap::AndroidRuntimeRequest request{};
    if (db_path != nullptr && db_path[0] != '\0') {
      request.db_path = fs::path(db_path);
    }
    request.output_root = fs::path(output_root);
    request.converter_config_toml_path = fs::path(converter_config_toml_path);

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      throw std::runtime_error("failed to create core runtime.");
    }

    auto* handle = new TtCoreRuntimeHandle();
    handle->runtime = std::move(runtime);
    return handle;
  } catch (const std::exception& error) {
    SetLastError(error.what());
    return nullptr;
  } catch (...) {
    SetLastError("tracer_core_runtime_create failed unexpectedly.");
    return nullptr;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

extern "C" TT_CORE_API void tracer_core_runtime_destroy(
    TtCoreRuntimeHandle* handle) {
  delete handle;
}
