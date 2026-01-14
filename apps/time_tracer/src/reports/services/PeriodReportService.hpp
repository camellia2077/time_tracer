// reports/services/PeriodReportService.hpp
#ifndef PERIOD_REPORT_SERVICE_HPP
#define PERIOD_REPORT_SERVICE_HPP

#include <sqlite3.h>
#include <vector>
#include "reports/shared/model/query_data_structs.hpp"
#include "reports/shared/types/ReportFormat.hpp"
// [修复] 更新包含路径
#include "common/config/AppConfig.hpp"

class PeriodReportService {
public:
    explicit PeriodReportService(sqlite3* db, const AppConfig& config);

    /**
     * @brief 为指定的周期列表生成所有报告。
     * * 优化说明：
     * 采用了批量获取策略 (Batch Fetching)，对于包含多个周期（如 7, 30, 90）的列表，
     * 只会执行一次针对最大周期的数据库查询，然后内存过滤。
     */
    FormattedPeriodReports generate_reports(const std::vector<int>& days_list, ReportFormat format);

private:
    sqlite3* db_;
    const AppConfig& app_config_;
};

#endif // PERIOD_REPORT_SERVICE_HPP
