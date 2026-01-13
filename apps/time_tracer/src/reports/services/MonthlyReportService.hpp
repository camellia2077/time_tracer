// reports/services/MonthlyReportService.hpp
#ifndef MONTHLY_REPORT_SERVICE_HPP
#define MONTHLY_REPORT_SERVICE_HPP

#include <sqlite3.h>
#include <map>
#include <string>
#include "reports/shared/model/query_data_structs.hpp"
#include "reports/shared/types/ReportFormat.hpp"
#include "common/AppConfig.hpp"

class MonthlyReportService {
public:
    explicit MonthlyReportService(sqlite3* db, const AppConfig& config);

    /**
     * @brief 生成所有历史月份的报告。
     * * 优化说明：
     * 内部不再循环调用 MonthQuerier，而是使用 SQL 聚合查询一次性获取所有数据，
     * 避免了 N+1 查询问题，显著提升生成速度。
     */
    FormattedMonthlyReports generate_reports(ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_;
};

#endif // MONTHLY_REPORT_SERVICE_HPP