// infrastructure/config/static_converter_config_provider.hpp
#ifndef INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_
#define INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_

#include "application/ports/i_converter_config_provider.hpp"

namespace infrastructure::config {

class StaticConverterConfigProvider final
    : public tracer_core::application::ports::IConverterConfigProvider {
 public:
  explicit StaticConverterConfigProvider(ConverterConfig converter_config);

  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override;

 private:
  ConverterConfig converter_config_;
};

}  // namespace infrastructure::config

#endif  // INFRASTRUCTURE_CONFIG_STATIC_CONVERTER_CONFIG_PROVIDER_H_
