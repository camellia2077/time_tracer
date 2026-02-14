// api/cli/impl/app/app_runner.cpp
#include "api/cli/impl/app/app_runner.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "api/cli/impl/app/cli_application.hpp"
#include "api/cli/impl/utils/console_helper.hpp"
#include "application/ports/i_cli_runtime_factory.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/version.hpp"

namespace AppRunner {

namespace {

using namespace ConsoleHelper;

// [Refactor] Validates and handles information-only commands (tracer, motto,
// version). Returns an exit code if handled, or std::nullopt if execution
// should continue.
auto HandleInfoCommands(const std::vector<std::string>& args)
    -> std::optional<int> {
  if (args.size() < 2) {
    return std::nullopt;
  }

  const auto& cmd = args[1];

  if (cmd == "tracer") {
    SafePrintln("\n{}{}{}{}\n", time_tracer::common::colors::kCyan,
                time_tracer::common::colors::kItalic,
                "  \"Cheers, love! The timetracer is here.\"",
                time_tracer::common::colors::kReset);
    return 0;
  }

  if (cmd == "motto" || cmd == "zen") {
    SafePrintln("");
    SafePrintln("{}{}{}  \"Trace your time, log your life.\"{}\n",
                time_tracer::common::colors::kCyan,
                time_tracer::common::colors::kItalic,
                time_tracer::common::colors::kBold,
                time_tracer::common::colors::kReset);
    return 0;
  }

  if (cmd == "-v" || cmd == "--version") {
    SafePrintln("{}{}TimeMaster Command Version: {}{}",
                time_tracer::common::colors::kBrightGreen,
                time_tracer::common::colors::kBold, AppInfo::kVersion,
                time_tracer::common::colors::kReset);
    SafePrintln("{}Last Updated:  {}{}", time_tracer::common::colors::kGray,
                AppInfo::kLastUpdated, time_tracer::common::colors::kReset);

    return 0;
  }

  return std::nullopt;
}

auto ValidateEnv(
    const std::filesystem::path& exe_path, bool is_help_mode,
    const std::shared_ptr<time_tracer::application::ports::ICliRuntimeFactory>&
        kRuntimeFactory) -> bool {
  if (!kRuntimeFactory) {
    SafePrintln(std::cerr, "\n{}CLI runtime factory is not available.{}\n",
                time_tracer::common::colors::kRed,
                time_tracer::common::colors::kReset);
    return false;
  }

  if (!kRuntimeFactory->ValidateEnvironment(exe_path, is_help_mode)) {
    SafePrintln(std::cerr,
                "\n{}Configuration validation failed. Please check the errors "
                "above.{}\n",
                time_tracer::common::colors::kRed,
                time_tracer::common::colors::kReset);
    return false;
  }
  return true;
}

}  // namespace

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

  const auto kRuntimeFactory =
      time_tracer::application::ports::CreateCliRuntimeFactory();

  if (!ValidateEnv(args[0], is_help_mode, kRuntimeFactory)) {
    return 1;
  }

  CliApplication controller(args, kRuntimeFactory);
  return controller.Execute();
}

}  // namespace AppRunner
