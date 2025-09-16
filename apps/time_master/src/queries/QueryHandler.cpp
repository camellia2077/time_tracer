// queries/QueryHandler.cpp
#include "QueryHandler.hpp"
#include "daily/DayGenerator.hpp"
#include "monthly/MonthGenerator.hpp"
#include "period/PeriodGenerator.hpp"
#include "export/AllDayReports.hpp"
#include "export/AllMonthlyReports.hpp"
#include "export/AllPeriodReports.hpp"

QueryHandler::QueryHandler(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string QueryHandler::run_daily_query(const std::string& date, ReportFormat format) const {
    DayGenerator generator(m_db, app_config_.day_typ_config_path, app_config_.day_md_config_path, app_config_.day_tex_config_path);
    return generator.generate_report(date, format);
}

std::string QueryHandler::run_monthly_query(const std::string& month, ReportFormat format) const {
    // [FIXED] The constructor now correctly takes the AppConfig object.
    MonthGenerator generator(m_db, app_config_);
    return generator.generate_report(month, format);
}

std::string QueryHandler::run_period_query(int days, ReportFormat format) const {
    PeriodGenerator generator(m_db, app_config_.period_typ_config_path, app_config_.period_md_config_path);
    return generator.generate_report(days, format);
}

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
    AllDayReports generator(m_db, app_config_.day_typ_config_path, app_config_.day_md_config_path, app_config_.day_tex_config_path);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports QueryHandler::run_export_all_monthly_reports_query(ReportFormat format) const {
    // [FIXED] The constructor now correctly takes the AppConfig object.
    AllMonthlyReports generator(m_db, app_config_);
    return generator.generate_reports(format);
}

FormattedPeriodReports QueryHandler::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    AllPeriodReports generator(m_db, app_config_.period_typ_config_path, app_config_.period_md_config_path);
    return generator.generate_reports(days_list, format);
}