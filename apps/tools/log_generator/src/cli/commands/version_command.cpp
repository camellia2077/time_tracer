// cli/commands/version_command.cpp
#include "cli/commands/version_command.hpp"

#include <iostream>

#include "version.hpp"

namespace CliCommands {

void PrintVersion() {
  std::cout << "log_generator version " << AppVersion::APP_VERSION << std::endl;
  std::cout << "Last Updated: " << AppVersion::LAST_UPDATE << std::endl;
}

}  // namespace CliCommands
