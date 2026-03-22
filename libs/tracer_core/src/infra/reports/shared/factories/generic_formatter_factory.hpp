// infra/reports/shared/factories/generic_formatter_factory.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
#define INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>

#include "domain/reports/types/report_types.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/reports/shared/interfaces/i_report_formatter.hpp"

template <typename ReportDataType>
class GenericFormatterFactory {
 public:
  using Creator =
      std::function<std::unique_ptr<IReportFormatter<ReportDataType>>(
          const ReportCatalog&)>;

  [[nodiscard]] static auto Create(ReportFormat format,
                                   const ReportCatalog& catalog)
      -> std::unique_ptr<IReportFormatter<ReportDataType>> {
    auto& creators = GetCreators();
    auto iter = creators.find(format);

    if (iter == creators.end()) {
      throw std::invalid_argument(
          "Unsupported report format or formatter not registered for this data "
          "type.");
    }

    return iter->second(catalog);
  }

  static void RegisterCreator(ReportFormat format, Creator creator) {
    GetCreators()[format] = std::move(creator);
  }

  static auto GetCreators() -> std::map<ReportFormat, Creator>& {
    static std::map<ReportFormat, Creator> creators;
    return creators;
  }
};

#endif  // INFRASTRUCTURE_REPORTS_SHARED_FACTORIES_GENERIC_FORMATTER_FACTORY_H_
