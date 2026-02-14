// infrastructure/reports/services/yearly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

class YearlyReportService {
 public:
  explicit YearlyReportService(sqlite3* sqlite_db,
                               const ReportCatalog& report_catalog);

  auto GenerateReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_YEARLY_REPORT_SERVICE_H_
