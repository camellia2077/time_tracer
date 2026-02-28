// infrastructure/reports/facade/report_formatter_registry_adapter.cpp
#include "infrastructure/reports/facade/report_formatter_registry_adapter.hpp"

#include <memory>
#include <utility>

#include "infrastructure/reports/formatter_registry.hpp"
#include "infrastructure/reports/plugin_manifest.hpp"

namespace {

class NoopStaticReportFormatterRegistrar final
    : public tracer_core::application::ports::IStaticReportFormatterRegistrar {
 public:
  auto RegisterStaticFormatters() const -> void override {}
};

}  // namespace

namespace infrastructure::reports {

ReportFormatterRegistryAdapter::ReportFormatterRegistryAdapter(
    std::shared_ptr<
        tracer_core::application::ports::IStaticReportFormatterRegistrar>
        static_registrar)
    : static_registrar_(std::move(static_registrar)) {}

auto ReportFormatterRegistryAdapter::RegisterFormatters() const -> void {
  ::reports::RegisterReportFormatters();
  static_registrar_->RegisterStaticFormatters();
}

auto ReportFormatterRegistryAdapter::GetExpectedFormatterPluginNames() const
    -> std::vector<std::string> {
  return ::reports::plugin_manifest::GetExpectedFormatterPluginNames();
}

}  // namespace infrastructure::reports

namespace tracer_core::application::ports {

auto CreateReportFormatterRegistry(
    std::shared_ptr<IStaticReportFormatterRegistrar> static_registrar)
    -> std::shared_ptr<IReportFormatterRegistry> {
  if (!static_registrar) {
    static_registrar = std::make_shared<NoopStaticReportFormatterRegistrar>();
  }

  return std::make_shared<
      ::infrastructure::reports::ReportFormatterRegistryAdapter>(
      std::move(static_registrar));
}

}  // namespace tracer_core::application::ports
