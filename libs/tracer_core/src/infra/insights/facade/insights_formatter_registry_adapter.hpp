// infra/insights/facade/insights_formatter_registry_adapter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_FACADE_INSIGHTS_FORMATTER_REGISTRY_ADAPTER_H_
#define INFRASTRUCTURE_INSIGHTS_FACADE_INSIGHTS_FORMATTER_REGISTRY_ADAPTER_H_

#include <memory>

#include "application/ports/insights/i_insights_formatter_registry.hpp"

namespace infrastructure::insights {

class InsightsFormatterRegistryAdapter final
    : public tracer_core::application::ports::IInsightsFormatterRegistry {
 public:
  explicit InsightsFormatterRegistryAdapter(
      std::shared_ptr<
          tracer_core::application::ports::IStaticInsightsFormatterRegistrar>
          static_registrar);

  auto RegisterFormatters() const -> void override;

 private:
  std::shared_ptr<
      tracer_core::application::ports::IStaticInsightsFormatterRegistrar>
      static_registrar_;
};

}  // namespace infrastructure::insights

#endif  // INFRASTRUCTURE_INSIGHTS_FACADE_INSIGHTS_FORMATTER_REGISTRY_ADAPTER_H_
