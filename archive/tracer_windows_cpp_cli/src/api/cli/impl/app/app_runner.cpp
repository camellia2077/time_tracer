// api/cli/impl/app/app_runner.cpp
#include "api/cli/impl/app/app_runner.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

struct ThirdPartyLicenseEntry {
  std::string_view name;
  std::string_view component;
  std::string_view version;
  std::string_view license_id;
  std::string_view homepage;
  std::string_view license_url;
  std::string_view notes;
};

constexpr std::string_view kProjectLicense = "Apache License 2.0";
constexpr std::string_view kProjectLicenseUrl =
    "https://www.apache.org/licenses/LICENSE-2.0";

constexpr std::array<ThirdPartyLicenseEntry, 10> kThirdPartyLicenses = {
    ThirdPartyLicenseEntry{
        .name = "nlohmann/json",
        .component = "cpp_cli",
        .version = "3.12.0",
        .license_id = "MIT",
        .homepage = "https://github.com/nlohmann/json",
        .license_url = "https://github.com/nlohmann/json/blob/develop/LICENSE."
                       "MIT",
        .notes = "C++ JSON parser used by the Windows C++ CLI and transport "
                 "layer.",
    },
    ThirdPartyLicenseEntry{
        .name = "Apache ECharts",
        .component = "shared/assets",
        .version = "5.x (vendored minified build)",
        .license_id = "Apache-2.0",
        .homepage = "https://echarts.apache.org/",
        .license_url = "https://www.apache.org/licenses/LICENSE-2.0",
        .notes = "Bundled chart rendering JS asset for report-chart export.",
    },
    ThirdPartyLicenseEntry{
        .name = "clap",
        .component = "rust_cli",
        .version = "4.5",
        .license_id = "Apache-2.0 OR MIT",
        .homepage = "https://github.com/clap-rs/clap",
        .license_url = "https://github.com/clap-rs/clap/blob/master/LICENSE",
        .notes = "Rust CLI argument parsing.",
    },
    ThirdPartyLicenseEntry{
        .name = "thiserror",
        .component = "rust_cli",
        .version = "2.0",
        .license_id = "Apache-2.0 OR MIT",
        .homepage = "https://github.com/dtolnay/thiserror",
        .license_url =
            "https://github.com/dtolnay/thiserror/blob/master/LICENSE-MIT",
        .notes = "Rust CLI error definitions.",
    },
    ThirdPartyLicenseEntry{
        .name = "libloading",
        .component = "rust_cli",
        .version = "0.8",
        .license_id = "ISC",
        .homepage = "https://github.com/nagisa/rust_libloading",
        .license_url =
            "https://github.com/nagisa/rust_libloading/blob/master/LICENSE",
        .notes = "Rust dynamic library loading helper.",
    },
    ThirdPartyLicenseEntry{
        .name = "serde",
        .component = "rust_cli",
        .version = "1.0",
        .license_id = "Apache-2.0 OR MIT",
        .homepage = "https://github.com/serde-rs/serde",
        .license_url = "https://github.com/serde-rs/serde/blob/master/LICENSE-"
                       "MIT",
        .notes = "Rust serialization framework.",
    },
    ThirdPartyLicenseEntry{
        .name = "serde_json",
        .component = "rust_cli",
        .version = "1.0",
        .license_id = "Apache-2.0 OR MIT",
        .homepage = "https://github.com/serde-rs/json",
        .license_url = "https://github.com/serde-rs/json/blob/master/LICENSE-"
                       "MIT",
        .notes = "Rust JSON encoding/decoding.",
    },
    ThirdPartyLicenseEntry{
        .name = "toml",
        .component = "rust_cli",
        .version = "0.8",
        .license_id = "Apache-2.0 OR MIT",
        .homepage = "https://github.com/toml-rs/toml",
        .license_url =
            "https://github.com/toml-rs/toml/blob/main/crates/toml/LICENSE",
        .notes = "Rust TOML parsing support.",
    },
    ThirdPartyLicenseEntry{
        .name = "SQLite",
        .component = "tracer_core runtime",
        .version = "3.51.2",
        .license_id = "Public Domain",
        .homepage = "https://sqlite.org/",
        .license_url = "https://sqlite.org/copyright.html",
        .notes = "Embedded database engine used by tracer_core runtime.",
    },
    ThirdPartyLicenseEntry{
        .name = "Zstandard (zstd)",
        .component = "tracer_core runtime",
        .version = "1.x",
        .license_id = "BSD-3-Clause",
        .homepage = "https://github.com/facebook/zstd",
        .license_url = "https://github.com/facebook/zstd/blob/dev/LICENSE",
        .notes = "Compression backend used for encrypted transfer payloads.",
    },
};

[[nodiscard]] auto IsHelpModeCommand(std::string_view command) -> bool {
  return command == "-h" || command == "--help";
}

void PrintProjectLicenseInfo() {
  SafePrintln("TimeTracer project license: {}", kProjectLicense);
  SafePrintln("License URL: {}", kProjectLicenseUrl);
  SafePrintln(
      "Run `licenses` to view third-party dependency license information.");
}

void PrintThirdPartyLicensesUsage(std::string_view executable_name) {
  SafePrintln("Usage: {} licenses [--full]", executable_name);
  SafePrintln("  licenses          Print summary list (default).");
  SafePrintln("  licenses --full   Print detailed third-party license info.");
}

void PrintThirdPartyLicensesSummary() {
  SafePrintln("Third-party licenses (summary):");
  for (const auto &entry : kThirdPartyLicenses) {
    SafePrintln("  - {} [{}] {} ({})", entry.name, entry.component,
                entry.license_id, entry.version);
  }
  SafePrintln("Run `licenses --full` for detailed third-party license info.");
}

void PrintThirdPartyLicensesFull() {
  SafePrintln("Third-party licenses (full):");
  for (const auto &entry : kThirdPartyLicenses) {
    SafePrintln("");
    SafePrintln("- Name: {}", entry.name);
    SafePrintln("  Component: {}", entry.component);
    SafePrintln("  Version: {}", entry.version);
    SafePrintln("  License: {}", entry.license_id);
    SafePrintln("  Homepage: {}", entry.homepage);
    SafePrintln("  License URL: {}", entry.license_url);
    SafePrintln("  Notes: {}", entry.notes);
  }
}

auto HandleThirdPartyLicensesCommand(const std::vector<std::string> &args)
    -> int {
  if (args.empty()) {
    return static_cast<int>(AppExitCode::kInvalidArguments);
  }

  bool emit_full = false;
  for (size_t i = 2; i < args.size(); ++i) {
    const auto &option = args[i];
    if (option == "--help" || option == "-h") {
      PrintThirdPartyLicensesUsage(args.front());
      return static_cast<int>(AppExitCode::kSuccess);
    }
    if (option == "--full") {
      emit_full = true;
      continue;
    }
    SafePrintln(std::cerr,
                "{}Invalid Argument: Unknown option '{}' for "
                "'licenses'.{}",
                tracer_core::common::colors::kRed, option,
                tracer_core::common::colors::kReset);
    PrintThirdPartyLicensesUsage(args.front());
    return static_cast<int>(AppExitCode::kInvalidArguments);
  }

  if (emit_full) {
    PrintThirdPartyLicensesFull();
    return static_cast<int>(AppExitCode::kSuccess);
  }

  PrintThirdPartyLicensesSummary();
  return static_cast<int>(AppExitCode::kSuccess);
}

void PrintVersionInfo() {
  SafePrintln("{}{}Tracer Windows CLI Version: {}{}",
              tracer_core::common::colors::kBrightGreen,
              tracer_core::common::colors::kBold,
              TracerWindowsVersion::kCliVersion,
              tracer_core::common::colors::kReset);
  SafePrintln("{}Core Version: {}{}", tracer_core::common::colors::kGray,
              TracerWindowsVersion::kCoreVersion,
              tracer_core::common::colors::kReset);
  SafePrintln("{}CLI Build Time (UTC): {}{}",
              tracer_core::common::colors::kGray,
              TracerWindowsVersion::kCliBuildTimestamp,
              tracer_core::common::colors::kReset);
}

// Handles information-only commands (tracer, motto, zen, version/license).
// Returns an exit code if handled, or std::nullopt if execution should
// continue.
auto HandleInfoCommands(const std::vector<std::string> &args)
    -> std::optional<int> {
  if (args.size() < 2) {
    return std::nullopt;
  }

  const auto &cmd = args[1];

  if (cmd == "tracer") {
    SafePrintln("\n{}{}{}{}\n", tracer_core::common::colors::kCyan,
                tracer_core::common::colors::kItalic,
                "  \"Cheers, love! The timetracer is here.\"",
                tracer_core::common::colors::kReset);
    return 0;
  }

  if (cmd == "motto" || cmd == "zen") {
    SafePrintln("");
    SafePrintln("{}{}{}  \"Trace your time, log your life.\"{}\n",
                tracer_core::common::colors::kCyan,
                tracer_core::common::colors::kItalic,
                tracer_core::common::colors::kBold,
                tracer_core::common::colors::kReset);
    return 0;
  }

  if (cmd == "-v" || cmd == "--version") {
    PrintVersionInfo();
    return 0;
  }

  if (cmd == "--license") {
    if (args.size() > 2) {
      SafePrintln(std::cerr,
                  "{}Invalid Argument: `--license` does not accept "
                  "extra arguments.{}",
                  tracer_core::common::colors::kRed,
                  tracer_core::common::colors::kReset);
      return static_cast<int>(AppExitCode::kInvalidArguments);
    }
    PrintProjectLicenseInfo();
    return 0;
  }

  if (cmd == "licenses") {
    return HandleThirdPartyLicensesCommand(args);
  }

  return std::nullopt;
}

void RewriteLegacyAlias(std::vector<std::string> &args, bool is_help_mode) {
  if (args.size() < 2 || is_help_mode) {
    return;
  }

  if (args[1] == "blink") {
    args[1] = "ingest";
  }
}

auto ValidateEnv(
    const std::filesystem::path &exe_path, bool is_help_mode,
    const std::shared_ptr<tracer_core::application::ports::ICliRuntimeFactory>
        &kRuntimeFactory) -> AppExitCode {
  using tracer_core::application::ports::CliRuntimeValidationFailure;

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
                tracer_core::common::colors::kRed,
                "CLI runtime factory is not available.",
                tracer_core::common::colors::kReset);
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

  const bool is_help_mode = IsHelpModeCommand(args[1]);

  if (auto exit_code = HandleInfoCommands(args); exit_code.has_value()) {
    return *exit_code;
  }

  RewriteLegacyAlias(args, is_help_mode);
  const bool is_doctor_mode = (args[1] == "doctor");

  const auto kRuntimeFactory =
      tracer_core::application::ports::CreateCliRuntimeFactory();

  const AppExitCode validation_exit_code =
      ValidateEnv(args[0], is_help_mode || is_doctor_mode, kRuntimeFactory);
  if (validation_exit_code != AppExitCode::kSuccess) {
    return static_cast<int>(validation_exit_code);
  }

  CliApplication controller(args, kRuntimeFactory);
  return controller.Execute();
}

} // namespace AppRunner
