// cli/impl/app/app_runner.cpp
#include "cli/impl/app/app_runner.hpp"

#include <iostream>
#include <optional>
#include <filesystem>

#include "cli/impl/app/cli_application.hpp"
#include "cli/impl/utils/console_helper.hpp"
#include "bootstrap/startup_validator.hpp"
#include "config/config_loader.hpp"
#include "common/config/app_config.hpp"
#include "common/ansi_colors.hpp"
#include "common/version.hpp"

namespace AppRunner {

namespace {

using namespace ConsoleHelper;

// [Refactor] Validates and handles information-only commands (tracer, motto, version).
// Returns an exit code if handled, or std::nullopt if execution should continue.
auto HandleInfoCommands(const std::vector<std::string>& args) -> std::optional<int> {
    if (args.size() < 2) {
        return std::nullopt;
    }

    const auto& cmd = args[1];

    if (cmd == "tracer") {
      SafePrintln("\n{}{}{}\n", time_tracer::common::colors::kCyan,
                  "  \"Cheers, love! The timetracer is here.\"", time_tracer::common::colors::kReset);
      return 0;
    }
    
    if (cmd == "motto" || cmd == "zen") {
      SafePrintln("");
      SafePrintln("{}  \"Trace your time, log your life.\"{}\n", time_tracer::common::colors::kCyan,
                  time_tracer::common::colors::kReset);
      return 0;
    }

    if (cmd == "-v" || cmd == "--version") {
      SafePrintln("TimeMaster Command Version: {}", AppInfo::kVersion);
      SafePrintln("Last Updated:  {}", AppInfo::kLastUpdated);

      return 0;
    }

    return std::nullopt;
}

// [Refactor] Loads configuration and validates the environment.
// Returns true if environment is valid (or help mode), false otherwise.
auto ValidateEnv(const std::string& exe_path, bool is_help_mode) -> bool {
    ConfigLoader boot_loader(exe_path);
    AppConfig config = boot_loader.LoadConfiguration();

    if (!is_help_mode) {
      if (!StartupValidator::ValidateEnvironment(config)) {

        SafePrintln(std::cerr,
                    "\n{}Configuration validation failed. Please check the "
                    "errors above.{}\n",
                    time_tracer::common::colors::kRed, time_tracer::common::colors::kReset);
        return false;
      }
    }
    return true;
}

} // namespace

auto Run(std::vector<std::string> args) -> int {
    if (args.size() < 2) {
      args.emplace_back("--help");
    }

    bool is_help_mode = (args[1] == "-h" || args[1] == "--help");

    // [Refactor] Handle info commands (tracer, motto, version) early
    if (auto exit_code = HandleInfoCommands(args); exit_code.has_value()) {
        return *exit_code;
    }

    if (!is_help_mode) {
      if (args[1] == "pre") {
        args[1] = "preprocess";
      }
      if (args[1] == "blink") {
        args[1] = "ingest";
      }
    }
    
    if (!ValidateEnv(args[0], is_help_mode)) {
        return 1;
    }

    CliApplication controller(args);
    return controller.Execute();
}

} // namespace AppRunner
