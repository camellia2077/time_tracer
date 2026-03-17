// api/c_api/tracer_core_c_api_runtime_config.cpp
#include "api/c_api/tracer_core_c_api_internal.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "api/c_api/cli_runtime_config_bridge.hpp"

namespace tracer_core::core::c_api::internal {

namespace fs = std::filesystem;
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
