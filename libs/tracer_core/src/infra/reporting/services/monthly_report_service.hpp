// infra/reporting/services/monthly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_

#include "infra/sqlite_fwd.hpp"

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infra/config/models/report_catalog.hpp"

namespace tracer::core::infrastructure::reports::services {
class MonthlyReportService {
 public:
  explicit MonthlyReportService(sqlite3* database_connection,
                                const ReportCatalog& report_catalog);

  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedMonthlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

}  // namespace tracer::core::infrastructure::reports::services

namespace infrastructure::reports::services {

using tracer::core::infrastructure::reports::services::MonthlyReportService;

}  // namespace infrastructure::reports::services

using MonthlyReportService =
    tracer::core::infrastructure::reports::services::MonthlyReportService;

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_
