#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "queries/report_generators/_shared/query_data_structs.h"
#include "queries/report_generators/_shared/ReportFormat.h" 

class QueryHandler {
public:
    explicit QueryHandler(sqlite3* db);

    // --- 单项查询 ---
    std::string run_daily_query(const std::string& date_str, ReportFormat format) const;
    // [修改] 为周期查询添加 ReportFormat 参数
    std::string run_period_query(int days, ReportFormat format) const;
    std::string run_monthly_query(const std::string& year_month_str, ReportFormat format) const;


    // --- 批量导出查询 ---
    FormattedGroupedReports run_export_all_daily_reports_query(ReportFormat format) const;
    FormattedMonthlyReports run_export_all_monthly_reports_query(ReportFormat format) const;
    // [修改] 为批量导出周期报告添加 ReportFormat 参数
    FormattedPeriodReports run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format = ReportFormat::Markdown) const;

private:
    sqlite3* m_db;
};

#endif // QUERY_HANDLER_H