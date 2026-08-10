// infra/insights/facade/insights_formatter_registry_adapter.cpp
#include "infra/insights/facade/insights_formatter_registry_adapter.hpp"

#include <memory>
#include <utility>

#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"

namespace infrastructure::insights {

InsightsFormatterRegistryAdapter::InsightsFormatterRegistryAdapter(
    std::shared_ptr<
        tracer_core::application::ports::IStaticInsightsFormatterRegistrar>
        static_registrar)
    : static_registrar_(std::move(static_registrar)) {}

auto InsightsFormatterRegistryAdapter::RegisterFormatters() const -> void {
  static_registrar_->RegisterStaticFormatters();
}

}  // namespace infrastructure::insights

namespace tracer_core::application::ports {

auto CreateInsightsFormatterRegistry(
    std::shared_ptr<IStaticInsightsFormatterRegistrar> static_registrar)
    -> std::shared_ptr<IInsightsFormatterRegistry> {
  if (!static_registrar) {
    static_registrar = std::make_shared<
        ::infrastructure::insights::AndroidStaticInsightsFormatterRegistrar>(
        ::infrastructure::insights::AndroidStaticInsightsFormatterPolicy::
            AllFormats());
  }

  return std::make_shared<
      ::infrastructure::insights::InsightsFormatterRegistryAdapter>(
      std::move(static_registrar));
}

}  // namespace tracer_core::application::ports
