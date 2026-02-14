// infrastructure/reports/facade/report_formatter_registry_adapter.hpp
#ifndef INFRASTRUCTURE_REPORTS_FACADE_REPORT_FORMATTER_REGISTRY_ADAPTER_H_
#define INFRASTRUCTURE_REPORTS_FACADE_REPORT_FORMATTER_REGISTRY_ADAPTER_H_

#include <memory>
#include <string>
#include <vector>

#include "application/ports/i_report_formatter_registry.hpp"

namespace infrastructure::reports {

class ReportFormatterRegistryAdapter final
    : public time_tracer::application::ports::IReportFormatterRegistry {
 public:
  explicit ReportFormatterRegistryAdapter(
      std::shared_ptr<
          time_tracer::application::ports::IStaticReportFormatterRegistrar>
          static_registrar);

  auto RegisterFormatters() const -> void override;

  [[nodiscard]] auto GetExpectedFormatterPluginNames() const
      -> std::vector<std::string> override;

 private:
  std::shared_ptr<
      time_tracer::application::ports::IStaticReportFormatterRegistrar>
      static_registrar_;
};

}  // namespace infrastructure::reports

#endif  // INFRASTRUCTURE_REPORTS_FACADE_REPORT_FORMATTER_REGISTRY_ADAPTER_H_
