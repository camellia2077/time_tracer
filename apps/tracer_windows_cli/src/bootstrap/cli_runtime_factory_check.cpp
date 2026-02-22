// tracer_windows/src/bootstrap/cli_runtime_factory_check.cpp
#include "bootstrap/cli_runtime_factory_check.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "domain/types/date_check_mode.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace time_tracer::cli::bootstrap::internal {

namespace fs = std::filesystem;

namespace {

using time_tracer::common::DllCompatibilityError;
using time_tracer::common::LogicError;
namespace tt_transport = tracer::transport;

constexpr std::string_view kCoreLibraryName = "time_tracer_core.dll";

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char code_point) -> char {
                   return static_cast<char>(std::tolower(code_point));
                 });
  return value;
}

[[nodiscard]] auto ParseDateCheckModeFromJsonString(const std::string &value)
    -> DateCheckMode {
  const std::string normalized = ToLowerAscii(value);
  if (normalized == "none") {
    return DateCheckMode::kNone;
  }
  if (normalized == "continuity") {
    return DateCheckMode::kContinuity;
  }
  if (normalized == "full") {
    return DateCheckMode::kFull;
  }
  throw LogicError("Unsupported date_check_mode: " + value);
}

[[nodiscard]] auto
ParseOptionalDateMode(const std::optional<std::string> &value)
    -> std::optional<DateCheckMode> {
  if (!value.has_value()) {
    return std::nullopt;
  }
  return ParseDateCheckModeFromJsonString(*value);
}

} // namespace

auto ValidateRequiredRuntimeFiles(const fs::path &bin_dir) -> bool {
  const fs::path core_library_path = bin_dir / kCoreLibraryName;
  if (!fs::exists(core_library_path)) {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed
              << "Runtime check failed: missing required runtime file: "
              << core_library_path.string() << colors::kReset << '\n';
    return false;
  }
  return true;
}

auto ParseRuntimeCheckResult(const char *payload_json, std::string_view context)
    -> RuntimeCheckResult {
  if (payload_json == nullptr || payload_json[0] == '\0') {
    throw LogicError(std::string(context) + ": empty JSON payload.");
  }

  try {
    const auto payload = tt_transport::DecodeRuntimeCheckResponse(payload_json);
    RuntimeCheckResult result{};
    result.ok = payload.ok;
    result.error_message = payload.error_message;
    result.messages = payload.messages;
    return result;
  } catch (const std::exception &error) {
    throw LogicError(std::string(context) +
                     ": invalid JSON payload: " + error.what());
  }
}

auto ParseResolvedCliContextFromCore(const char *payload_json)
    -> ResolvedCliContextFromCore {
  if (payload_json == nullptr || payload_json[0] == '\0') {
    throw DllCompatibilityError(
        "tracer_core_runtime_resolve_cli_context_json returned empty payload.");
  }

  tt_transport::ResolveCliContextResponsePayload decoded{};
  try {
    decoded = tt_transport::DecodeResolveCliContextResponse(payload_json);
  } catch (const std::exception &error) {
    throw DllCompatibilityError(
        std::string("Invalid resolve-cli-context payload: ") + error.what());
  }

  if (!decoded.ok) {
    throw DllCompatibilityError(
        "tracer_core_runtime_resolve_cli_context_json failed: " +
        (decoded.error_message.empty() ? std::string("unknown")
                                       : decoded.error_message));
  }
  if (!decoded.paths.has_value()) {
    throw DllCompatibilityError(
        "resolve-cli-context payload missing object field `paths`.");
  }
  if (!decoded.cli_config.has_value()) {
    throw DllCompatibilityError(
        "resolve-cli-context payload missing object field `cli_config`.");
  }

  const auto &paths = *decoded.paths;
  const auto &cli_config_payload = *decoded.cli_config;

  ResolvedCliContextFromCore context{};
  context.db_path = fs::path(paths.db_path);
  context.runtime_output_root = fs::path(paths.runtime_output_root);
  context.converter_config_toml_path =
      fs::path(paths.converter_config_toml_path);

  time_tracer::application::dto::CliConfig cli_config{};
  cli_config.default_save_processed_output =
      cli_config_payload.default_save_processed_output;
  if (cli_config_payload.default_date_check_mode.has_value()) {
    cli_config.default_date_check_mode = ParseDateCheckModeFromJsonString(
        *cli_config_payload.default_date_check_mode);
  }
  cli_config.defaults.default_format =
      cli_config_payload.defaults.default_format;
  cli_config.command_defaults.export_format =
      cli_config_payload.command_defaults.export_format;
  cli_config.command_defaults.query_format =
      cli_config_payload.command_defaults.query_format;
  cli_config.command_defaults.convert_date_check_mode = ParseOptionalDateMode(
      cli_config_payload.command_defaults.convert_date_check_mode);
  cli_config.command_defaults.convert_save_processed_output =
      cli_config_payload.command_defaults.convert_save_processed_output;
  cli_config.command_defaults.convert_validate_logic =
      cli_config_payload.command_defaults.convert_validate_logic;
  cli_config.command_defaults.convert_validate_structure =
      cli_config_payload.command_defaults.convert_validate_structure;
  cli_config.command_defaults.ingest_date_check_mode = ParseOptionalDateMode(
      cli_config_payload.command_defaults.ingest_date_check_mode);
  cli_config.command_defaults.ingest_save_processed_output =
      cli_config_payload.command_defaults.ingest_save_processed_output;
  cli_config.command_defaults.validate_logic_date_check_mode =
      ParseOptionalDateMode(
          cli_config_payload.command_defaults.validate_logic_date_check_mode);

  context.cli_config = std::move(cli_config);
  return context;
}

} // namespace time_tracer::cli::bootstrap::internal
