// tracer_windows/src/bootstrap/cli_runtime_factory_check.hpp
#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "application/dto/cli_config.hpp"

namespace time_tracer::cli::bootstrap::internal {

struct RuntimeCheckResult {
  bool ok = false;
  std::string error_message;
  std::vector<std::string> messages;
};

struct ResolvedCliContextFromCore {
  std::filesystem::path db_path;
  std::filesystem::path runtime_output_root;
  std::filesystem::path converter_config_toml_path;
  time_tracer::application::dto::CliConfig cli_config;
};

[[nodiscard]] auto
ValidateRequiredRuntimeFiles(const std::filesystem::path &bin_dir) -> bool;

[[nodiscard]] auto ParseRuntimeCheckResult(const char *payload_json,
                                           std::string_view context)
    -> RuntimeCheckResult;

[[nodiscard]] auto ParseResolvedCliContextFromCore(const char *payload_json)
    -> ResolvedCliContextFromCore;

} // namespace time_tracer::cli::bootstrap::internal
