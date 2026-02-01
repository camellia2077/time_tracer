// cli/commands/help_command.cpp
#include "cli/commands/help_command.hpp"

#include <iostream>

#include "common/ansi_colors.hpp"

namespace CliCommands {

void PrintError(std::string_view message) {
  std::cerr << RED_COLOR << "Error" << RESET_COLOR << ": " << message << "\n\n";
}

void PrintUsage(std::string_view prog_name) {
  std::cerr << GREEN_COLOR << "Usage: " << RESET_COLOR << prog_name
            << " [options]\n\n";
  std::cerr << "Description: Generates test log data for a given year or year "
               "range.\n";
  std::cerr << "             Reads configuration from "
               "'config/activities_config.toml'\n";
  std::cerr << "             and 'config/mapping_config.toml'.\n\n";
  std::cerr << GREEN_COLOR << "Options:\n" << RESET_COLOR;
  std::cerr << "  -y, --year <year>       Generate data for a single year.\n";
  std::cerr << "  -s, --start <year>      The starting year for a range. (Used "
               "with --end)\n";
  std::cerr << "  -e, --end <year>        The ending year for a range "
               "(inclusive). (Used with --start)\n";
  std::cerr << "  -i, --items <number>    Number of log items per day (must be "
               ">= 2). (Default: 10)\n";
  std::cerr << "  -n, --nosleep           Enable the generation of 'no sleep' "
               "(all-nighter) days.\n";
  std::cerr
      << "  -v, --version           Display version information and exit.\n";
  std::cerr
      << "  -h, --help              Display this help message and exit.\n\n";
  std::cerr << GREEN_COLOR << "Example:\n" << RESET_COLOR;
  std::cerr << "  " << prog_name << " --year 2025\n";
  std::cerr << "  " << prog_name
            << " --start 2024 --end 2025 --items 5 --nosleep\n";
}

}  // namespace CliCommands
