// application/bootstrap/startup_validator.cpp
#include "application/bootstrap/startup_validator.hpp"

#include "application/ports/i_runtime_environment_validator.hpp"
#include "application/ports/logger.hpp"
#include "shared/types/ansi_colors.hpp"

auto StartupValidator::ValidateEnvironment(
    const tracer_core::application::dto::RuntimeEnvironmentRequirements&
        requirements,
    const tracer_core::application::ports::IRuntimeEnvironmentValidator&
        validator) -> bool {
  const bool kPluginsOk = validator.ValidateFormatterPlugins(requirements);
  const bool kCoreOk = validator.ValidateCoreRuntimeLibraries(requirements);

  if (!kPluginsOk || !kCoreOk) {
    namespace colors = tracer_core::common::colors;
    tracer_core::application::ports::LogError(
        std::string(colors::kRed) +
        "Fatal: Runtime environment validation failed "
        "(missing or incompatible runtime DLLs)." +
        std::string(colors::kReset));
    return false;
  }

  return true;
}
