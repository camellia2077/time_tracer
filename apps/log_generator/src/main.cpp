// main.cpp
#include <filesystem>
#include <span>

#include "application/application.hpp"
#include "cli/commands/help_command.hpp"
#include "cli/commands/version_command.hpp"
#include "cli/framework/command_line_parser.hpp"
#include "domain/impl/log_generator_factory.hpp"
#include "infrastructure/io/file_manager.hpp"

auto main(int argc, char* argv[]) -> int {
  CommandLineParser parser(
      std::span<char* const>(argv, static_cast<size_t>(argc)));
  CliRequest request = parser.parse();

  if (request.action == CliAction::kHelp) {
    CliCommands::PrintUsage(parser.prog_name());
    return 0;
  }

  if (request.action == CliAction::kVersion) {
    CliCommands::PrintVersion();
    return 0;
  }

  if (request.action == CliAction::kError) {
    CliCommands::PrintError(request.error_message);
    CliCommands::PrintUsage(parser.prog_name());
    return 1;
  }

  if (!request.config) {
    return 1;
  }

  std::filesystem::path exe_dir;
  if (argc > 0 && argv != nullptr) {
    exe_dir = std::filesystem::path(argv[0]).parent_path();
  }

  FileManager file_manager;
  LogGeneratorFactory generator_factory;
  App::Application app(file_manager, generator_factory);
  return app.run(*request.config, exe_dir);
}
