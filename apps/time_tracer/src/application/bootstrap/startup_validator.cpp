// application/bootstrap/startup_validator.cpp
#include "application/bootstrap/startup_validator.hpp"

#include "application/ports/i_runtime_environment_validator.hpp"
#include "application/ports/logger.hpp"
#include "shared/types/ansi_colors.hpp"

auto StartupValidator::ValidateEnvironment(
    const time_tracer::application::dto::RuntimeEnvironmentRequirements&
        requirements,
    const time_tracer::application::ports::IRuntimeEnvironmentValidator&
        validator) -> bool {
  const bool kPluginsOk = validator.ValidateFormatterPlugins(requirements);
  const bool kCoreOk = validator.ValidateCoreRuntimeLibraries(requirements);

  if (!kPluginsOk || !kCoreOk) {
    namespace colors = time_tracer::common::colors;
    time_tracer::application::ports::LogError(
        std::string(colors::kRed) +
        "Fatal: Runtime environment validation failed "
        "(missing or incompatible runtime DLLs)." +
        std::string(colors::kReset));
    return false;
  }

  return true;
}
