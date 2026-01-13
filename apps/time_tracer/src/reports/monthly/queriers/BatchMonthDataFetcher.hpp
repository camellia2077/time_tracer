// reports/monthly/queriers/BatchMonthDataFetcher.hpp
#ifndef BATCH_MONTH_DATA_FETCHER_HPP
#define BATCH_MONTH_DATA_FETCHER_HPP

#include <sqlite3.h>
#include <map>
#include <string>
#include "reports/shared/model/MonthlyReportData.hpp"

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

#endif // BATCH_MONTH_DATA_FETCHER_HPP