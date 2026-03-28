module;

#include <utility>

#include "application/ports/pipeline/i_converter_config_provider.hpp"

module tracer.core.infrastructure.config.static_converter_config_provider;

namespace tracer::core::infrastructure::config {

StaticConverterConfigProvider::StaticConverterConfigProvider(
    ConverterConfig converter_config)
    : converter_config_(std::move(converter_config)) {}

auto StaticConverterConfigProvider::LoadConverterConfig() const
    -> ConverterConfig {
  return converter_config_;
}

auto StaticConverterConfigProvider::InvalidateCache() -> void {}

}  // namespace tracer::core::infrastructure::config
