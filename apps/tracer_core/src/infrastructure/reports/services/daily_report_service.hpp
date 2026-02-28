// infrastructure/reports/services/daily_report_service.hpp
#ifndef INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
#define INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <string>

#include "domain/reports/models/query_data_structs.hpp"
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"

class DailyReportService {
 public:
  /**
   * @brief 构造函数。
   * @param db
   * 指向数据库连接的指针。

   * *
   * @param config 应用程序配置对象的引用。
   */
  explicit DailyReportService(sqlite3* sqlite_db,
                              const ReportCatalog& report_catalog);

  /**
   * @brief 生成所有日报并返回分类好的结果。
   * @param format 需要生成的报告格式。
   * @return 一个包含所有格式化后日报的嵌套 map。
   */
  auto GenerateAllReports(ReportFormat format) -> FormattedGroupedReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

#endif  // INFRASTRUCTURE_REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
