// queries/QueryHandler.cpp
#include "QueryHandler.hpp"
#include "daily/DayGenerator.hpp"
#include "monthly/MonthGenerator.hpp"
#include "period/PeriodGenerator.hpp"
#include "export/AllDayReports.hpp"
#include "export/AllMonthlyReports.hpp"
#include "export/AllPeriodReports.hpp"

QueryHandler::QueryHandler(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string QueryHandler::run_daily_query(const std::string& date, ReportFormat format) const {
    // [修改] DayGenerator 现在接收 AppConfig
    DayGenerator generator(db_, app_config_);
    return generator.generate_report(date, format);
}

std::string QueryHandler::run_monthly_query(const std::string& month, ReportFormat format) const {
    MonthGenerator generator(db_, app_config_);
    return generator.generate_report(month, format);
}

std::string QueryHandler::run_period_query(int days, ReportFormat format) const {
    PeriodGenerator generator(db_, app_config_);
    return generator.generate_report(days, format);
}

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    // [修改] AllDayReports 现在接收 AppConfig
    AllDayReports generator(db_, app_config_);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    AllMonthlyReports generator(db_, app_config_);
    return generator.generate_reports(format);
}

FormattedPeriodReports QueryHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    AllPeriodReports generator(db_, app_config_); 
    return generator.generate_reports(days_list, format);
}