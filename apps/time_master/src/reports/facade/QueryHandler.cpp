// reports/facade/QueryHandler.cpp
#include "QueryHandler.hpp"

// [核心修改] 移除旧的 Generator 头文件，引入 BaseGenerator 和所有需要的 Querier/Data 头文件
#include "reports/shared/generators/BaseGenerator.hpp"
#include "reports/daily/DayQuerier.hpp"
#include "reports/monthly/MonthQuerier.hpp"
#include "reports/period/PeriodQuerier.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/shared/data/PeriodReportData.hpp"

#include "reports/export/AllDayReports.hpp"
#include "reports/export/AllMonthlyReports.hpp"
#include "reports/export/AllPeriodReports.hpp"

QueryHandler::QueryHandler(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string QueryHandler::run_daily_query(const std::string& date, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<DailyReportData, DayQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(date, format);
}

std::string QueryHandler::run_monthly_query(const std::string& month, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<MonthlyReportData, MonthQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(month, format);
}

std::string QueryHandler::run_period_query(int days, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_, app_config_);
    return generator.generate_report(days, format);
}

// --- 批量导出的部分保持不变 ---

FormattedGroupedReports QueryHandler::run_export_all_daily_reports_query(ReportFormat format) const {
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