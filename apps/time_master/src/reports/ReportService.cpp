// reports/ReportService.cpp
#include "ReportService.hpp"

#include "reports/shared/generators/BaseGenerator.hpp"
#include "reports/daily/DayQuerier.hpp"
#include "reports/monthly/MonthQuerier.hpp"
#include "reports/period/PeriodQuerier.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/shared/data/PeriodReportData.hpp"

#include "reports/services/AllDayReports.hpp"
#include "reports/services/AllMonthlyReports.hpp"
#include "reports/services/AllPeriodReports.hpp"

ReportService::ReportService(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string ReportService::run_daily_query(const std::string& date, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<DailyReportData, DayQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(date, format);
}

std::string ReportService::run_monthly_query(const std::string& month, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<MonthlyReportData, MonthQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(month, format);
}

std::string ReportService::run_period_query(int days, ReportFormat format) const {
    // [核心修改] 直接使用 BaseGenerator
    BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_, app_config_);
    return generator.generate_report(days, format);
}

// --- 批量导出的部分保持不变 ---

FormattedGroupedReports ReportService::run_export_all_daily_reports_query(ReportFormat format) const {
    AllDayReports generator(db_, app_config_);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports ReportService::run_export_all_monthly_reports_query(ReportFormat format) const {
    AllMonthlyReports generator(db_, app_config_);
    return generator.generate_reports(format);
}

FormattedPeriodReports ReportService::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    AllPeriodReports generator(db_, app_config_);
    return generator.generate_reports(days_list, format);
}