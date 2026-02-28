// infrastructure/reports/services/weekly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

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

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
