// queries/QueryHandler.cpp

#include "QueryHandler.hpp"
#include "queries/daily/DayGenerator.hpp" 
#include "queries/monthly/MonthGenerator.hpp" 
#include "queries/period/PeriodGenerator.hpp"
#include "queries/export/AllDayReports.hpp"
#include "queries/export/AllMonthlyReports.hpp"
#include "queries/export/AllPeriodReports.hpp"

QueryHandler::QueryHandler(sqlite3* db, const AppConfig& config) 
    : m_db(db), app_config_(config) {} // [MODIFIED] Constructor

std::string QueryHandler::run_daily_query(const std::string& date_str, ReportFormat format) const {
    // [MODIFIED] Pass the config path to the DayGenerator
    DayGenerator generator(m_db, app_config_.day_typ_config_path);
    return generator.generate_report(date_str, format);
}

std::string QueryHandler::run_monthly_query(const std::string& year_month_str, ReportFormat format) const {
    MonthGenerator generator(m_db);
    return generator.generate_report(year_month_str, format);
}

std::string QueryHandler::run_period_query(int days, ReportFormat format) const {
    PeriodGenerator generator(m_db);
    return generator.generate_report(days, format);
}

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    // [MODIFIED] Pass the config path to AllDayReports
    AllDayReports generator(m_db, app_config_.day_typ_config_path);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    AllMonthlyReports generator(m_db);
    return generator.generate_reports(format);
}

FormattedPeriodReports QueryHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    AllPeriodReports generator(m_db);
    return generator.generate_reports(days_list, format);
}