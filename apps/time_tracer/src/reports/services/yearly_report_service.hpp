// reports/services/yearly_report_service.hpp
#ifndef REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
#define REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "common/config/app_config.hpp"
#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

class YearlyReportService {
 public:
  explicit YearlyReportService(sqlite3* sqlite_db, const AppConfig& config);

  auto GenerateReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const AppConfig& app_config_;
};

#endif  // REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
