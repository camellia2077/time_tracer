// infrastructure/reports/services/yearly_report_service.hpp
#ifndef REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
#define REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_format.hpp"
#include "infrastructure/config/models/app_config.hpp"

class YearlyReportService {
 public:
  explicit YearlyReportService(sqlite3* sqlite_db, const AppConfig& config);

  auto GenerateReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const AppConfig& app_config_;
};

#endif  // REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
