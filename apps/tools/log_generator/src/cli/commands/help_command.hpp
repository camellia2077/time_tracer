// cli/commands/help_command.hpp
#ifndef CLI_COMMANDS_HELP_COMMAND_H_
#define CLI_COMMANDS_HELP_COMMAND_H_

#include <string_view>

namespace CliCommands {
void PrintUsage(std::string_view prog_name);
void PrintError(std::string_view message);
}  // namespace CliCommands

#endif  // CLI_COMMANDS_HELP_COMMAND_H_
