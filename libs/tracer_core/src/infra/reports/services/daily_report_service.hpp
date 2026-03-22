// infra/reports/services/daily_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infra/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {
class DailyReportService {
 public:
  explicit DailyReportService(sqlite3* sqlite_db,
                              const ReportCatalog& report_catalog);

  auto GenerateAllReports(ReportFormat format) -> FormattedGroupedReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

}  // namespace tracer::core::infrastructure::reports::services

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::DailyReportService;

}  // namespace infrastructure::reports::services

using DailyReportService =
    tracer::core::infrastructure::reports::services::DailyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

