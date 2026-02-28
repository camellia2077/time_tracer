// bootstrap/cli_runtime_factory.cpp
#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "application/ports/i_cli_runtime_factory.hpp"
#include "bootstrap/cli_runtime_factory_support.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"

namespace {

namespace fs = std::filesystem;
using tracer_core::application::ports::CliRuntimeValidationFailure;
using tracer_core::cli::bootstrap::internal::CliRuntimeState;
using tracer_core::cli::bootstrap::internal::CoreLibrary;
using tracer_core::cli::bootstrap::internal::ParseResolvedCliContextFromCore;
using tracer_core::cli::bootstrap::internal::ParseRuntimeCheckResult;
using tracer_core::cli::bootstrap::internal::ResolvedCliContextFromCore;
using tracer_core::cli::bootstrap::internal::RuntimeCheckResult;
using tracer_core::cli::bootstrap::internal::ValidateRequiredRuntimeFiles;
using tracer_core::common::DllCompatibilityError;

void PrintRuntimeCheckError(const std::string &message) {
  namespace colors = tracer_core::common::colors;
  std::cerr << colors::kRed << "Runtime check failed: " << message
            << colors::kReset << '\n';
}

[[nodiscard]] auto ToLowerAscii(std::string value) -> std::string {
  for (char &code_point : value) {
    code_point =
        static_cast<char>(std::tolower(static_cast<unsigned char>(code_point)));
  }
  return value;
}

[[nodiscard]] auto ContainsToken(std::string_view text, std::string_view token)
    -> bool {
  return text.find(token) != std::string_view::npos;
}

[[nodiscard]] auto
ClassifyValidationFailureFromMessage(std::string_view message)
    -> CliRuntimeValidationFailure {
  const std::string normalized = ToLowerAscii(std::string(message));
  const std::string_view text = normalized;

  if (ContainsToken(text, "config.toml") ||
      ContainsToken(text, "configuration file not found") ||
      ContainsToken(text, "configuration")) {
    return CliRuntimeValidationFailure::kConfigurationError;
  }

  if (ContainsToken(text, "missing required runtime file") ||
      ContainsToken(text, "tracer_core.dll") ||
      ContainsToken(text, "failed to load") || ContainsToken(text, "dll") ||
      ContainsToken(text, "symbol")) {
    return CliRuntimeValidationFailure::kRuntimeDependencyMissing;
  }

  if (ContainsToken(text, "invalid argument") ||
      ContainsToken(text, "unsupported")) {
    return CliRuntimeValidationFailure::kInvalidArguments;
  }

  if (ContainsToken(text, "i/o") || ContainsToken(text, "io error") ||
      ContainsToken(text, "path") || ContainsToken(text, "permission denied") ||
      ContainsToken(text, "access is denied")) {
    return CliRuntimeValidationFailure::kIoError;
  }

  return CliRuntimeValidationFailure::kUnknownError;
}

[[nodiscard]] auto
ValidationFailurePriority(CliRuntimeValidationFailure failure) -> int {
  switch (failure) {
  case CliRuntimeValidationFailure::kConfigurationError:
    return 5;
  case CliRuntimeValidationFailure::kRuntimeDependencyMissing:
    return 4;
  case CliRuntimeValidationFailure::kIoError:
    return 3;
  case CliRuntimeValidationFailure::kInvalidArguments:
    return 2;
  case CliRuntimeValidationFailure::kUnknownError:
    return 1;
  case CliRuntimeValidationFailure::kNone:
    return 0;
  }
  return 0;
}

[[nodiscard]] auto MergeValidationFailure(CliRuntimeValidationFailure current,
                                          CliRuntimeValidationFailure candidate)
    -> CliRuntimeValidationFailure {
  if (ValidationFailurePriority(candidate) >
      ValidationFailurePriority(current)) {
    return candidate;
  }
  return current;
}

class InfrastructureCliRuntimeFactory final
    : public tracer_core::application::ports::ICliRuntimeFactory {
public:
  [[nodiscard]] auto ValidateEnvironment(const fs::path &executable_path,
                                         bool is_help_mode) const
      -> bool override {
    last_validation_failure_ = CliRuntimeValidationFailure::kNone;

    if (is_help_mode) {
      return true;
    }

    try {
      const fs::path executable = fs::absolute(executable_path);
      const fs::path bin_dir = executable.parent_path();
      if (!ValidateRequiredRuntimeFiles(bin_dir)) {
        last_validation_failure_ =
            CliRuntimeValidationFailure::kRuntimeDependencyMissing;
        return false;
      }

      const auto library = CoreLibrary::LoadFromExeDir(bin_dir);
      const char *payload_json =
          library->symbols().runtime_check_environment_json(
              executable.string().c_str(), 0);
      const RuntimeCheckResult result =
          ParseRuntimeCheckResult(payload_json, "runtime_check_environment");
      if (!result.ok) {
        CliRuntimeValidationFailure failure =
            CliRuntimeValidationFailure::kUnknownError;
        if (!result.messages.empty()) {
          for (const auto &message : result.messages) {
            PrintRuntimeCheckError(message);
            failure = MergeValidationFailure(
                failure, ClassifyValidationFailureFromMessage(message));
          }
        } else {
          const std::string message = result.error_message.empty()
                                          ? "Runtime environment check failed."
                                          : result.error_message;
          PrintRuntimeCheckError(message);
          failure = ClassifyValidationFailureFromMessage(message);
        }
        last_validation_failure_ = failure;
        return false;
      }
      return true;
    } catch (const std::exception &error) {
      PrintRuntimeCheckError(error.what());
      last_validation_failure_ =
          ClassifyValidationFailureFromMessage(std::string_view(error.what()));
      if (last_validation_failure_ == CliRuntimeValidationFailure::kNone) {
        last_validation_failure_ = CliRuntimeValidationFailure::kUnknownError;
      }
      return false;
    }
  }

  [[nodiscard]] auto GetLastValidationFailure() const
      -> CliRuntimeValidationFailure override {
    return last_validation_failure_;
  }

  [[nodiscard]] auto BuildRuntime(
      const tracer_core::application::ports::CliRuntimeRequest &request) const
      -> tracer_core::application::ports::CliRuntime override {
    using tracer_core::application::ports::CliRuntime;

    const fs::path executable = fs::absolute(request.executable_path);
    auto library = CoreLibrary::LoadFromExeDir(executable.parent_path());

    std::string db_override_utf8;
    const char *db_override_arg = nullptr;
    if (request.db_override.has_value()) {
      db_override_utf8 = fs::absolute(*request.db_override).string();
      db_override_arg = db_override_utf8.c_str();
    }

    std::string output_override_utf8;
    const char *output_override_arg = nullptr;
    if (request.output_override.has_value()) {
      output_override_utf8 = fs::absolute(*request.output_override).string();
      output_override_arg = output_override_utf8.c_str();
    }

    const char *command_name_arg =
        request.command_name.empty() ? nullptr : request.command_name.c_str();
    const char *resolve_payload_json =
        library->symbols().runtime_resolve_cli_context_json(
            executable.string().c_str(), db_override_arg, output_override_arg,
            command_name_arg);
    const ResolvedCliContextFromCore resolved =
        ParseResolvedCliContextFromCore(resolve_payload_json);

    std::error_code io_error;
    fs::create_directories(resolved.runtime_output_root, io_error);
    fs::create_directories(resolved.db_path.parent_path(), io_error);

    TtCoreRuntimeHandle *runtime_handle = library->symbols().runtime_create(
        resolved.db_path.string().c_str(),
        resolved.runtime_output_root.string().c_str(),
        resolved.converter_config_toml_path.string().c_str());
    if (runtime_handle == nullptr) {
      const char *error_message = library->symbols().last_error();
      throw DllCompatibilityError(
          "Failed to create core runtime: " +
          std::string(error_message != nullptr ? error_message : "unknown"));
    }

    CliRuntime runtime;
    auto proxy = tracer_core::cli::bootstrap::internal::MakeCoreApiProxy(
        library, runtime_handle);
    runtime.core_api = proxy;
    runtime.cli_config = resolved.cli_config;

    auto state = std::make_shared<CliRuntimeState>();
    state->library = std::move(library);
    state->core_api = proxy;
    runtime.runtime_state = std::move(state);
    return runtime;
  }

private:
  mutable CliRuntimeValidationFailure last_validation_failure_ =
      CliRuntimeValidationFailure::kNone;
};

} // namespace

namespace tracer_core::application::ports {

auto CreateCliRuntimeFactory() -> std::shared_ptr<ICliRuntimeFactory> {
  return std::make_shared<InfrastructureCliRuntimeFactory>();
}

} // namespace tracer_core::application::ports
