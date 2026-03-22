// infra/reports/services/yearly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infra/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {
class YearlyReportService {
 public:
  explicit YearlyReportService(sqlite3* sqlite_db,
                               const ReportCatalog& report_catalog);

  auto GenerateReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

}  // namespace tracer::core::infrastructure::reports::services

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::YearlyReportService;

}  // namespace infrastructure::reports::services

using YearlyReportService =
    tracer::core::infrastructure::reports::services::YearlyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_

