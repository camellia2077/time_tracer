// infrastructure/config/validator/plugins/facade/runtime_environment_validator_adapter.cpp
#include "infrastructure/config/validator/plugins/facade/runtime_environment_validator_adapter.hpp"

#include "infrastructure/config/validator/plugins/facade/plugin_validator.hpp"

namespace infrastructure::config::validator::plugins {

auto RuntimeEnvironmentValidatorAdapter::ValidateFormatterPlugins(
    const tracer_core::application::dto::RuntimeEnvironmentRequirements&
        requirements) const -> bool {
  return PluginValidator::Validate(requirements.plugins_directory,
                                   requirements.expected_formatter_plugins);
}

auto RuntimeEnvironmentValidatorAdapter::ValidateCoreRuntimeLibraries(
    const tracer_core::application::dto::RuntimeEnvironmentRequirements&
        requirements) const -> bool {
  PluginValidationOptions options{};
  options.require_formatter_abi = false;
  return PluginValidator::Validate(requirements.binary_directory,
                                   requirements.required_core_runtime_libraries,
                                   options);
}

}  // namespace infrastructure::config::validator::plugins
