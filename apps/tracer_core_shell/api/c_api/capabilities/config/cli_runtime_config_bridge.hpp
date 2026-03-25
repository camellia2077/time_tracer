#ifndef API_CORE_CLI_RUNTIME_CONFIG_BRIDGE_H_
#define API_CORE_CLI_RUNTIME_CONFIG_BRIDGE_H_

#include <filesystem>
#include <optional>
#include <string>

#include "domain/types/date_check_mode.hpp"

namespace tracer_core::shell::config_bridge {

struct CliGlobalDefaultsSnapshot {
  std::optional<std::filesystem::path> db_path;
  std::optional<std::filesystem::path> output_root;
  std::optional<std::string> default_format;
};

struct CliCommandDefaultsSnapshot {
  std::optional<std::string> export_format;
  std::optional<std::string> query_format;
  std::optional<DateCheckMode> convert_date_check_mode;
  std::optional<bool> convert_save_processed_output;
  std::optional<bool> convert_validate_logic;
  std::optional<bool> convert_validate_structure;
  std::optional<DateCheckMode> ingest_date_check_mode;
  std::optional<bool> ingest_save_processed_output;
  std::optional<DateCheckMode> validate_logic_date_check_mode;
};

struct CliConfigSnapshot {
  std::filesystem::path exe_dir_path;
  std::optional<std::filesystem::path> export_path;
  std::filesystem::path converter_config_toml_path;
  bool default_save_processed_output = false;
  DateCheckMode default_date_check_mode = DateCheckMode::kNone;
  CliGlobalDefaultsSnapshot defaults;
  CliCommandDefaultsSnapshot command_defaults;
};

[[nodiscard]] auto LoadCliConfigSnapshot(
    const std::filesystem::path& executable_path) -> CliConfigSnapshot;

}  // namespace tracer_core::shell::config_bridge

#endif  // API_CORE_CLI_RUNTIME_CONFIG_BRIDGE_H_
