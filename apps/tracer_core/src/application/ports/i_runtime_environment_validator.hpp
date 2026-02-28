// application/ports/i_runtime_environment_validator.hpp
#ifndef APPLICATION_PORTS_I_RUNTIME_ENVIRONMENT_VALIDATOR_H_
#define APPLICATION_PORTS_I_RUNTIME_ENVIRONMENT_VALIDATOR_H_

#include "application/dto/runtime_environment_requirements.hpp"

namespace tracer_core::application::ports {

class IRuntimeEnvironmentValidator {
 public:
  virtual ~IRuntimeEnvironmentValidator() = default;

  [[nodiscard]] virtual auto ValidateFormatterPlugins(
      const tracer_core::application::dto::RuntimeEnvironmentRequirements&
          requirements) const -> bool = 0;

  [[nodiscard]] virtual auto ValidateCoreRuntimeLibraries(
      const tracer_core::application::dto::RuntimeEnvironmentRequirements&
          requirements) const -> bool = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_RUNTIME_ENVIRONMENT_VALIDATOR_H_
