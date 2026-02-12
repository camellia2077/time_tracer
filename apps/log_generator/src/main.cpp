// main.cpp
#include <exception>
#include <filesystem>
#include <iostream>
#include <span>

#include "application/application.hpp"
#include "cli/commands/help_command.hpp"
#include "cli/commands/version_command.hpp"
#include "cli/framework/command_line_parser.hpp"
#include "common/exit_code.hpp"
#include "domain/impl/log_generator_factory.hpp"
#include "infrastructure/io/file_manager.hpp"

auto main(int argc, char* argv[]) -> int {
  try {
    CommandLineParser parser(
        std::span<char* const>(argv, static_cast<size_t>(argc)));
    CliRequest request = parser.parse();

    if (request.action == CliAction::kHelp) {
      CliCommands::PrintUsage(parser.prog_name());
      return App::to_status_code(App::ExitCode::kSuccess);
    }

    if (request.action == CliAction::kVersion) {
      CliCommands::PrintVersion();
      return App::to_status_code(App::ExitCode::kSuccess);
    }

    if (request.action == CliAction::kError) {
      CliCommands::PrintError(request.error_message);
      CliCommands::PrintUsage(parser.prog_name());
      return App::to_status_code(App::ExitCode::kCliError);
    }

    if (!request.config) {
      return App::to_status_code(App::ExitCode::kMissingRequestConfig);
    }

    std::filesystem::path exe_dir;
    if (argc > 0 && argv != nullptr) {
      exe_dir = std::filesystem::path(argv[0]).parent_path();
    }

    FileManager file_manager;
    LogGeneratorFactory generator_factory;
    App::Application app(file_manager, generator_factory);
    const App::ExitCode kExitCode = app.run(*request.config, exe_dir);
    return App::to_status_code(kExitCode);
  } catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return App::to_status_code(App::ExitCode::kInternalError);
  } catch (...) {
    std::cerr << "Fatal error: unknown exception.\n";
    return App::to_status_code(App::ExitCode::kInternalError);
  }
}
