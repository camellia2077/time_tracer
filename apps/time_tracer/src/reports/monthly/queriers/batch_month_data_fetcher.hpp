// reports/monthly/queriers/batch_month_data_fetcher.hpp
#ifndef REPORTS_MONTHLY_QUERIERS_BATCH_MONTH_DATA_FETCHER_HPP_
#define REPORTS_MONTHLY_QUERIERS_BATCH_MONTH_DATA_FETCHER_HPP_

#include <sqlite3.h>
#include <map>
#include <string>
#include "reports/shared/model/monthly_report_data.hpp"

/**
 * @class BatchMonthDataFetcher
 * @brief 专用于批量获取所有月份的报告数据。
 * * 职责：执行聚合 SQL 查询，解决 N+1 问题，将原始数据库行转换为 MonthlyReportData 对象。
 */
class BatchMonthDataFetcher {
public:
    explicit BatchMonthDataFetcher(sqlite3* db);

    /**
     * @brief 获取所有月份的数据。
     * @return map<"YYYY-MM", MonthlyReportData>
     */
    std::map<std::string, MonthlyReportData> fetch_all_data();

private:
    sqlite3* db_;

    // 内部辅助方法
    void fetch_project_stats(std::map<std::string, MonthlyReportData>& all_months_data);
    void fetch_actual_days(std::map<std::string, MonthlyReportData>& all_months_data);
};

#endif // REPORTS_MONTHLY_QUERIERS_BATCH_MONTH_DATA_FETCHER_HPP_