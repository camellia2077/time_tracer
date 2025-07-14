#ifndef QUERY_HANDLER_H
#define QUERY_HANDLER_H

#include <sqlite3.h>
#include <string>
#include "report_generators/_shared/query_data_structs.h"
class QueryHandler {
public:
    explicit QueryHandler(sqlite3* db);

    // 单项查询
    std::string run_daily_query(const std::string& date_str) const;
    std::string run_period_query(int days) const;
    std::string run_monthly_query(const std::string& year_month_str) const;

    // 批量导出查询
    FormattedGroupedReports run_export_all_daily_reports_query() const;
    FormattedMonthlyReports run_export_all_monthly_reports_query() const; // [新增]

private:
    sqlite3* m_db;
};

#endif // QUERY_HANDLER_H
