// cli/framework/cli_request.hpp
#ifndef CLI_FRAMEWORK_CLI_REQUEST_H_
#define CLI_FRAMEWORK_CLI_REQUEST_H_

#include <optional>
#include <string>

#include "common/config_types.hpp"

enum class CliAction { kRun, kHelp, kVersion, kError };

struct CliRequest {
  CliAction action = CliAction::kError;
  std::optional<Config> config;
  std::string error_message;
};

#endif  // CLI_FRAMEWORK_CLI_REQUEST_H_
