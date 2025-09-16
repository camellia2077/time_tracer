// queries/QueryHandler.hpp
#ifndef QUERY_HANDLER_HPP
#define QUERY_HANDLER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include "queries/shared/data/query_data_structs.hpp"
#include "queries/shared/ReportFormat.hpp" 
#include "common/AppConfig.hpp" // [ADDED] Include AppConfig

class QueryHandler {
public:
    explicit QueryHandler(sqlite3* db, const AppConfig& config); // [MODIFIED] Constructor

    // --- Single Queries ---
    std::string run_daily_query(const std::string& date_str, ReportFormat format) const;
    std::string run_period_query(int days, ReportFormat format) const;
    std::string run_monthly_query(const std::string& year_month_str, ReportFormat format) const;


    // --- Bulk Export Queries ---
    FormattedGroupedReports run_export_all_daily_reports_query(ReportFormat format) const;
    FormattedMonthlyReports run_export_all_monthly_reports_query(ReportFormat format) const;
    FormattedPeriodReports run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format = ReportFormat::Markdown) const;

private:
    sqlite3* m_db;
    const AppConfig& app_config_; // [ADDED] Store a reference to the config
};

#endif // QUERY_HANDLER_HPP