// reports/services/daily_report_service.hpp
#ifndef REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
#define REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_

#include <sqlite3.h>

#include <string>

#include "common/config/app_config.hpp"
#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

class DailyReportService {
 public:
  /**
   * @brief 构造函数。
   * @param db 指向数据库连接的指针。
   * @param config 应用程序配置对象的引用。
   */
  explicit DailyReportService(sqlite3* db, const AppConfig& config);

  /**
   * @brief 生成所有日报并返回分类好的结果。
   * @param format 需要生成的报告格式。
   * @return 一个包含所有格式化后日报的嵌套 map。
   */
  FormattedGroupedReports generate_all_reports(ReportFormat format);

 private:
  sqlite3* db_;
  const AppConfig& app_config_;
};

#endif  // REPORTS_SERVICES_DAILY_REPORT_SERVICE_H_
