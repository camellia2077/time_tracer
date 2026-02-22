// api/cli/impl/app/app_runner.cpp
#include "api/cli/impl/app/app_runner.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "api/cli/impl/app/cli_application.hpp"
#include "api/cli/impl/app/version.hpp"
#include "api/cli/impl/utils/console_helper.hpp"
#include "application/ports/i_cli_runtime_factory.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exit_codes.hpp"

namespace AppRunner {

namespace {

using namespace ConsoleHelper;

// [Refactor] Validates and handles information-only commands (tracer, motto,
// version). Returns an exit code if handled, or std::nullopt if execution
// should continue.
auto HandleInfoCommands(const std::vector<std::string> &args)
    -> std::optional<int> {
  if (args.size() < 2) {
    return std::nullopt;
  }

  const auto &cmd = args[1];

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
    SafePrintln("{}{}Tracer Windows CLI Version: {}{}",
                time_tracer::common::colors::kBrightGreen,
                time_tracer::common::colors::kBold,
                TracerWindowsVersion::kCliVersion,
                time_tracer::common::colors::kReset);
    SafePrintln("{}Core Version: {}{}", time_tracer::common::colors::kGray,
                TracerWindowsVersion::kCoreVersion,
                time_tracer::common::colors::kReset);
    SafePrintln("{}CLI Build Time (UTC): {}{}",
                time_tracer::common::colors::kGray,
                TracerWindowsVersion::kCliBuildTimestamp,
                time_tracer::common::colors::kReset);

    return 0;
  }

  return std::nullopt;
}

auto ValidateEnv(
    const std::filesystem::path &exe_path, bool is_help_mode,
    const std::shared_ptr<time_tracer::application::ports::ICliRuntimeFactory>
        &kRuntimeFactory) -> AppExitCode {
  using time_tracer::application::ports::CliRuntimeValidationFailure;

  auto map_validation_failure_to_exit_code =
      [](const CliRuntimeValidationFailure failure) -> AppExitCode {
    switch (failure) {
    case CliRuntimeValidationFailure::kNone:
      return AppExitCode::kSuccess;
    case CliRuntimeValidationFailure::kRuntimeDependencyMissing:
      return AppExitCode::kDllCompatibilityError;
    case CliRuntimeValidationFailure::kConfigurationError:
      return AppExitCode::kConfigError;
    case CliRuntimeValidationFailure::kIoError:
      return AppExitCode::kIoError;
    case CliRuntimeValidationFailure::kInvalidArguments:
      return AppExitCode::kInvalidArguments;
    case CliRuntimeValidationFailure::kUnknownError:
      return AppExitCode::kGenericError;
    }
    return AppExitCode::kGenericError;
  };

  if (!kRuntimeFactory) {
    SafePrintln(std::cerr, "{}Runtime bootstrap failed: {}{}",
                time_tracer::common::colors::kRed,
                "CLI runtime factory is not available.",
                time_tracer::common::colors::kReset);
    return AppExitCode::kDllCompatibilityError;
  }

  if (!kRuntimeFactory->ValidateEnvironment(exe_path, is_help_mode)) {
    // Runtime factory already prints concrete diagnostics (missing files,
    // DLL load failures, runtime-check details). Do not wrap again here.
    return map_validation_failure_to_exit_code(
        kRuntimeFactory->GetLastValidationFailure());
  }
  return AppExitCode::kSuccess;
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

  const auto kRuntimeFactory =
      time_tracer::application::ports::CreateCliRuntimeFactory();

  const AppExitCode validation_exit_code =
      ValidateEnv(args[0], is_help_mode, kRuntimeFactory);
  if (validation_exit_code != AppExitCode::kSuccess) {
    return static_cast<int>(validation_exit_code);
  }

  CliApplication controller(args, kRuntimeFactory);
  return controller.Execute();
}

} // namespace AppRunner
