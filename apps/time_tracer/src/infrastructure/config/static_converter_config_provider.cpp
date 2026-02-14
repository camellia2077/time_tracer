// infrastructure/config/static_converter_config_provider.cpp
#include "infrastructure/config/static_converter_config_provider.hpp"

#include <utility>

namespace infrastructure::config {

StaticConverterConfigProvider::StaticConverterConfigProvider(
    ConverterConfig converter_config)
    : converter_config_(std::move(converter_config)) {}

auto StaticConverterConfigProvider::LoadConverterConfig() const
    -> ConverterConfig {
  return converter_config_;
}

}  // namespace infrastructure::config
