// infrastructure/config/validator/plugins/facade/runtime_environment_validator_adapter.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_RUNTIME_ENVIRONMENT_VALIDATOR_ADAPTER_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_RUNTIME_ENVIRONMENT_VALIDATOR_ADAPTER_H_

#include "application/ports/i_runtime_environment_validator.hpp"

namespace infrastructure::config::validator::plugins {

class RuntimeEnvironmentValidatorAdapter final
    : public time_tracer::application::ports::IRuntimeEnvironmentValidator {
 public:
  [[nodiscard]] auto ValidateFormatterPlugins(
      const time_tracer::application::dto::RuntimeEnvironmentRequirements&
          requirements) const -> bool override;

  [[nodiscard]] auto ValidateCoreRuntimeLibraries(
      const time_tracer::application::dto::RuntimeEnvironmentRequirements&
          requirements) const -> bool override;
};

}  // namespace infrastructure::config::validator::plugins

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_RUNTIME_ENVIRONMENT_VALIDATOR_ADAPTER_H_
