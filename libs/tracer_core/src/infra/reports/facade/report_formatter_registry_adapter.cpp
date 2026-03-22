// infra/reports/facade/report_formatter_registry_adapter.cpp
#include "infra/reports/facade/report_formatter_registry_adapter.hpp"

#include <memory>
#include <utility>

#include "infra/reports/facade/android_static_report_formatter_registrar.hpp"

namespace infrastructure::reports {

ReportFormatterRegistryAdapter::ReportFormatterRegistryAdapter(
    std::shared_ptr<
        tracer_core::application::ports::IStaticReportFormatterRegistrar>
        static_registrar)
    : static_registrar_(std::move(static_registrar)) {}

auto ReportFormatterRegistryAdapter::RegisterFormatters() const -> void {
  static_registrar_->RegisterStaticFormatters();
}

}  // namespace infrastructure::reports

namespace tracer_core::application::ports {

auto CreateReportFormatterRegistry(
    std::shared_ptr<IStaticReportFormatterRegistrar> static_registrar)
    -> std::shared_ptr<IReportFormatterRegistry> {
  if (!static_registrar) {
    static_registrar = std::make_shared<
        ::infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
        ::infrastructure::reports::AndroidStaticReportFormatterPolicy::
            AllFormats());
  }

  return std::make_shared<
      ::infrastructure::reports::ReportFormatterRegistryAdapter>(
      std::move(static_registrar));
}

}  // namespace tracer_core::application::ports
