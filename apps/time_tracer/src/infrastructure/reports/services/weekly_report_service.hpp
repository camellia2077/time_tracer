// infrastructure/reports/services/weekly_report_service.hpp
#ifndef REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
#define REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_format.hpp"
#include "infrastructure/config/models/app_config.hpp"

class WeeklyReportService {
 public:
  explicit WeeklyReportService(sqlite3* database_connection,
                               const AppConfig& config);

  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedWeeklyReports;

 private:
  sqlite3* db_;
  const AppConfig& app_config_;
};

#endif  // REPORTS_SERVICES_WEEKLY_REPORT_SERVICE_H_
