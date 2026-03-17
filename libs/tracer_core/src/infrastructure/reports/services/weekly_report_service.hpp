// infrastructure/reports/services/weekly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

#include "infrastructure/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {
class WeeklyReportService {
 public:
  explicit WeeklyReportService(sqlite3* database_connection,
                               const ReportCatalog& report_catalog);

  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedWeeklyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

}  // namespace tracer::core::infrastructure::reports::services

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::WeeklyReportService;

}  // namespace infrastructure::reports::services

using WeeklyReportService =
    tracer::core::infrastructure::reports::services::WeeklyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

