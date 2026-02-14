// application/dto/cli_config.hpp
#ifndef APPLICATION_DTO_CLI_CONFIG_H_
#define APPLICATION_DTO_CLI_CONFIG_H_

#include <optional>
#include <string>

#include "domain/types/date_check_mode.hpp"

namespace time_tracer::application::dto {

struct CliGlobalDefaults {
  std::optional<std::string> default_format;
};

struct CliCommandDefaults {
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

struct CliConfig {
  bool default_save_processed_output = false;
  DateCheckMode default_date_check_mode = DateCheckMode::kNone;
  CliGlobalDefaults defaults;
  CliCommandDefaults command_defaults;
};

}  // namespace time_tracer::application::dto

#endif  // APPLICATION_DTO_CLI_CONFIG_H_
