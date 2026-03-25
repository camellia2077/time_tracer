// api/c_api/capabilities/config/tracer_core_c_api_runtime_config.cpp
#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/c_api/capabilities/config/cli_runtime_config_bridge.hpp"

using tracer_core::core::c_api::internal::ClearLastError;
using tracer_core::core::c_api::internal::ResolvedCliContext;

namespace {

namespace fs = std::filesystem;
using nlohmann::json;

constexpr std::string_view kCoreLibraryName = "tracer_core.dll";
constexpr std::string_view kReportsSharedLibraryName = "reports_shared.dll";
constexpr std::string_view kSqliteLibraryName = "libsqlite3-0.dll";
constexpr std::string_view kTomlLibraryName = "libtomlplusplus-3.dll";
constexpr std::string_view kLibgccRuntimeName = "libgcc_s_seh-1.dll";
constexpr std::string_view kLibstdcppRuntimeName = "libstdc++-6.dll";
constexpr std::string_view kLibwinpthreadRuntimeName = "libwinpthread-1.dll";

#ifndef TT_RUNTIME_REQUIRE_SQLITE_DLL
#define TT_RUNTIME_REQUIRE_SQLITE_DLL 1
#endif

#ifndef TT_RUNTIME_REQUIRE_TOML_DLL
#define TT_RUNTIME_REQUIRE_TOML_DLL 1
#endif

#ifndef TT_RUNTIME_REQUIRE_MINGW_DLLS
#define TT_RUNTIME_REQUIRE_MINGW_DLLS 1
#endif

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

[[nodiscard]] auto BuildCliConfigJson(
    const tracer_core::core::c_api::internal::CliConfigContext& cli_config)
    -> json {
  return json{
      {"default_save_processed_output",
       cli_config.default_save_processed_output},
      {"default_date_check_mode",
       std::string(ToDateCheckModeString(cli_config.default_date_check_mode))},
      {"defaults",
       {{"default_format", ToStringJson(cli_config.defaults.default_format)}}},
      {"command_defaults",
       {{"export_format", ToStringJson(cli_config.command_defaults.export_format)},
        {"query_format", ToStringJson(cli_config.command_defaults.query_format)},
        {"convert_date_check_mode",
         ToDateCheckModeJson(
             cli_config.command_defaults.convert_date_check_mode)},
        {"convert_save_processed_output",
         cli_config.command_defaults.convert_save_processed_output.has_value()
             ? json(*cli_config.command_defaults.convert_save_processed_output)
             : json(nullptr)},
        {"convert_validate_logic",
         cli_config.command_defaults.convert_validate_logic.has_value()
             ? json(*cli_config.command_defaults.convert_validate_logic)
             : json(nullptr)},
        {"convert_validate_structure",
         cli_config.command_defaults.convert_validate_structure.has_value()
             ? json(*cli_config.command_defaults.convert_validate_structure)
             : json(nullptr)},
        {"ingest_date_check_mode",
         ToDateCheckModeJson(
             cli_config.command_defaults.ingest_date_check_mode)},
        {"ingest_save_processed_output",
         cli_config.command_defaults.ingest_save_processed_output.has_value()
             ? json(*cli_config.command_defaults.ingest_save_processed_output)
             : json(nullptr)},
        {"validate_logic_date_check_mode",
             ToDateCheckModeJson(
             cli_config.command_defaults.validate_logic_date_check_mode)}}}};
}

[[nodiscard]] auto BuildFailureJsonResponse(
    std::string_view error_message,
    const std::vector<std::string>& messages = {},
    std::string_view error_code = "runtime.generic_error",
    std::string_view error_category = "runtime",
    std::vector<std::string> hints = {}) -> const char* {
  const std::string kNormalizedError = error_message.empty()
                                           ? std::string("Unknown error.")
                                           : std::string(error_message);
  tracer_core::core::c_api::internal::SetLastError(kNormalizedError.c_str());

  if (hints.empty()) {
    hints.emplace_back("Inspect `error_message` for detailed failure reason.");
  }
  json payload = {{"ok", false},
                  {"error_message", kNormalizedError},
                  {"error_code", std::string(error_code)},
                  {"error_category", std::string(error_category)},
                  {"hints", std::move(hints)}};
  if (!messages.empty()) {
    payload["messages"] = messages;
  }
  tracer_core::core::c_api::internal::g_last_response = payload.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

[[nodiscard]] auto BuildSuccessJsonResponse(const json& payload) -> const char* {
  json normalized = payload;
  normalized["error_code"] = "";
  normalized["error_category"] = "";
  normalized["hints"] = json::array();
  tracer_core::core::c_api::internal::g_last_response = normalized.dump();
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

}  // namespace

namespace tracer_core::core::c_api::internal {
namespace shell_config_bridge = tracer_core::shell::config_bridge;

namespace {

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";

[[nodiscard]] auto IsNonEmptyCString(const char* value) -> bool {
  return value != nullptr && value[0] != '\0';
}

[[nodiscard]] auto ToCliGlobalDefaultsContext(
    const shell_config_bridge::CliGlobalDefaultsSnapshot& defaults)
    -> CliGlobalDefaultsContext {
  return {
      .db_path = defaults.db_path,
      .output_root = defaults.output_root,
      .default_format = defaults.default_format,
  };
}

[[nodiscard]] auto ToCliCommandDefaultsContext(
    const shell_config_bridge::CliCommandDefaultsSnapshot& defaults)
    -> CliCommandDefaultsContext {
  return {
      .export_format = defaults.export_format,
      .query_format = defaults.query_format,
      .convert_date_check_mode = defaults.convert_date_check_mode,
      .convert_save_processed_output = defaults.convert_save_processed_output,
      .convert_validate_logic = defaults.convert_validate_logic,
      .convert_validate_structure = defaults.convert_validate_structure,
      .ingest_date_check_mode = defaults.ingest_date_check_mode,
      .ingest_save_processed_output = defaults.ingest_save_processed_output,
      .validate_logic_date_check_mode =
          defaults.validate_logic_date_check_mode,
  };
}

[[nodiscard]] auto ToCliConfigContext(
    const shell_config_bridge::CliConfigSnapshot& cli_config)
    -> CliConfigContext {
  return {
      .exe_dir_path = cli_config.exe_dir_path,
      .export_path = cli_config.export_path,
      .converter_config_toml_path = cli_config.converter_config_toml_path,
      .default_save_processed_output =
          cli_config.default_save_processed_output,
      .default_date_check_mode = cli_config.default_date_check_mode,
      .defaults = ToCliGlobalDefaultsContext(cli_config.defaults),
      .command_defaults =
          ToCliCommandDefaultsContext(cli_config.command_defaults),
  };
}

}  // namespace

auto ResolveCliContext(const char* executable_path, const char* db_override,
                       const char* output_override, const char* command_name)
    -> ResolvedCliContext {
  if (!IsNonEmptyCString(executable_path)) {
    throw std::invalid_argument("executable_path must not be empty.");
  }

  const fs::path executable = fs::absolute(fs::path(executable_path));
  CliConfigContext cli_config = ToCliConfigContext(
      shell_config_bridge::LoadCliConfigSnapshot(executable));

  fs::path output_root = fs::absolute(cli_config.exe_dir_path / "output");
  if (cli_config.defaults.output_root.has_value()) {
    output_root = fs::absolute(*cli_config.defaults.output_root);
  }

  fs::path export_root;
  if (IsNonEmptyCString(output_override)) {
    export_root = fs::absolute(fs::path(output_override));
  } else if (cli_config.export_path.has_value()) {
    export_root = fs::absolute(*cli_config.export_path);
  }

  fs::path db_path = output_root / "db" / kDatabaseFilename;
  if (cli_config.defaults.db_path.has_value()) {
    db_path = fs::absolute(*cli_config.defaults.db_path);
  }
  if (IsNonEmptyCString(db_override)) {
    db_path = fs::absolute(fs::path(db_override));
  }

  const std::string resolved_command =
      IsNonEmptyCString(command_name) ? std::string(command_name)
                                      : std::string();
  const fs::path runtime_output_root =
      (resolved_command == "export" && !export_root.empty()) ? export_root
                                                             : output_root;

  return {
      .cli_config = std::move(cli_config),
      .output_root = std::move(output_root),
      .db_path = std::move(db_path),
      .export_root = std::move(export_root),
      .runtime_output_root = runtime_output_root,
  };
}

}  // namespace tracer_core::core::c_api::internal

extern "C" TT_CORE_API auto tracer_core_runtime_check_environment_json(
    const char* executable_path, int is_help_mode) -> const char* {
  try {
    ClearLastError();
    const ResolvedCliContext kContext =
        tracer_core::core::c_api::internal::ResolveCliContext(
            executable_path, nullptr, nullptr, nullptr);
    if (is_help_mode != 0) {
      return BuildSuccessJsonResponse(json{
          {"ok", true},
          {"error_message", ""},
          {"messages", json::array()},
      });
    }

    const fs::path kBinDir = kContext.cli_config.exe_dir_path;
    std::vector<std::string> errors;
    std::vector<fs::path> required_files = {
        kBinDir / kCoreLibraryName,
        kBinDir / kReportsSharedLibraryName,
        kBinDir / "config" / "config.toml",
    };
    if (TT_RUNTIME_REQUIRE_SQLITE_DLL != 0) {
      required_files.emplace_back(kBinDir / kSqliteLibraryName);
    }
    if (TT_RUNTIME_REQUIRE_TOML_DLL != 0) {
      required_files.emplace_back(kBinDir / kTomlLibraryName);
    }
    if (TT_RUNTIME_REQUIRE_MINGW_DLLS != 0) {
      required_files.emplace_back(kBinDir / kLibgccRuntimeName);
      required_files.emplace_back(kBinDir / kLibstdcppRuntimeName);
      required_files.emplace_back(kBinDir / kLibwinpthreadRuntimeName);
    }

    for (const auto& path : required_files) {
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
    const ResolvedCliContext kContext =
        tracer_core::core::c_api::internal::ResolveCliContext(
            executable_path, db_override, output_override, command_name);
    return BuildSuccessJsonResponse(
        json{{"ok", true},
             {"error_message", ""},
             {"paths",
              {{"exe_dir", kContext.cli_config.exe_dir_path.string()},
               {"db_path", kContext.db_path.string()},
               {"output_root", kContext.output_root.string()},
               {"export_root", kContext.export_root.string()},
               {"runtime_output_root", kContext.runtime_output_root.string()},
               {"converter_config_toml_path",
                kContext.cli_config.converter_config_toml_path.string()}}},
             {"cli_config", BuildCliConfigJson(kContext.cli_config)}});
  } catch (const std::exception& error) {
    return BuildFailureJsonResponse(error.what(), {}, "config.resolve_failed",
                                    "config");
  } catch (...) {
    return BuildFailureJsonResponse(
        "tracer_core_runtime_resolve_cli_context_json failed unexpectedly.", {},
        "config.resolve_failed", "config");
  }
}
