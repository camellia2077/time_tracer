#include "api/c_api/capabilities/config/cli_runtime_config_bridge.hpp"

import tracer.core.infrastructure.config.internal.cli_config_snapshot;

namespace tracer_core::shell::config_bridge {

namespace modconfig_internal = tracer::core::infrastructure::modconfig::internal;

namespace {

[[nodiscard]] auto ToCliGlobalDefaultsSnapshot(
    const modconfig_internal::CliGlobalDefaultsSnapshot& defaults)
    -> CliGlobalDefaultsSnapshot {
  return {
      .db_path = defaults.db_path,
      .output_root = defaults.output_root,
      .default_format = defaults.default_format,
  };
}

[[nodiscard]] auto ToCliCommandDefaultsSnapshot(
    const modconfig_internal::CliCommandDefaultsSnapshot& defaults)
    -> CliCommandDefaultsSnapshot {
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

}  // namespace

auto LoadCliConfigSnapshot(const std::filesystem::path& executable_path)
    -> CliConfigSnapshot {
  const auto config = modconfig_internal::LoadCliConfigSnapshotCached(
      executable_path);
  return {
      .exe_dir_path = config.exe_dir_path,
      .export_path = config.export_path,
      .converter_config_toml_path = config.converter_config_toml_path,
      .default_save_processed_output = config.default_save_processed_output,
      .default_date_check_mode = config.default_date_check_mode,
      .defaults = ToCliGlobalDefaultsSnapshot(config.defaults),
      .command_defaults = ToCliCommandDefaultsSnapshot(config.command_defaults),
  };
}

}  // namespace tracer_core::shell::config_bridge
