// application/ports/i_converter_config_provider.hpp
#ifndef APPLICATION_PORTS_I_CONVERTER_CONFIG_PROVIDER_H_
#define APPLICATION_PORTS_I_CONVERTER_CONFIG_PROVIDER_H_

#include "domain/types/converter_config.hpp"

namespace time_tracer::application::ports {

class IConverterConfigProvider {
 public:
  virtual ~IConverterConfigProvider() = default;

  [[nodiscard]] virtual auto LoadConverterConfig() const -> ConverterConfig = 0;
};

}  // namespace time_tracer::application::ports

#endif  // APPLICATION_PORTS_I_CONVERTER_CONFIG_PROVIDER_H_
