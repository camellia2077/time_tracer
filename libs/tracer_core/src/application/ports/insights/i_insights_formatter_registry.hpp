// application/ports/insights/i_insights_formatter_registry.hpp
#ifndef APPLICATION_PORTS_I_INSIGHTS_FORMATTER_REGISTRY_H_
#define APPLICATION_PORTS_I_INSIGHTS_FORMATTER_REGISTRY_H_

#include <memory>

namespace tracer_core::application::ports {

class IStaticInsightsFormatterRegistrar {
 public:
  virtual ~IStaticInsightsFormatterRegistrar() = default;

  virtual auto RegisterStaticFormatters() const -> void = 0;
};

class IInsightsFormatterRegistry {
 public:
  virtual ~IInsightsFormatterRegistry() = default;

  virtual auto RegisterFormatters() const -> void = 0;
};

// Default formatter registry provider. Implemented in infrastructure.
auto CreateInsightsFormatterRegistry(
    std::shared_ptr<IStaticInsightsFormatterRegistrar> static_registrar =
        nullptr) -> std::shared_ptr<IInsightsFormatterRegistry>;

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_INSIGHTS_FORMATTER_REGISTRY_H_
