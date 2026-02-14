// infrastructure/reports/services/monthly_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <map>
#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

class MonthlyReportService {
 public:
  explicit MonthlyReportService(sqlite3* database_connection,
                                const ReportCatalog& report_catalog);

  /**
   * @brief 生成所有历史月份的报告。
   * * 优化说明：
   * 内部不再循环调用 MonthQuerier，而是使用 SQL 聚合查询一次性获取所有数据，
   * 避免了 N+1 查询问题，显著提升生成速度。
   */
  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedMonthlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_MONTHLY_REPORT_SERVICE_H_
