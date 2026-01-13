// reports/ReportService.cpp
#include "ReportService.hpp"

#include "reports/shared/generators/BaseGenerator.hpp"
#include "reports/daily/queriers/DayQuerier.hpp"
#include "reports/monthly/queriers/MonthQuerier.hpp"
#include "reports/period/queriers/PeriodQuerier.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/shared/model/MonthlyReportData.hpp"
#include "reports/shared/model/PeriodReportData.hpp"

#include "reports/services/DailyReportService.hpp"
#include "reports/services/MonthlyReportService.hpp"
#include "reports/services/PeriodReportService.hpp"

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
    DailyReportService generator(db_, app_config_);
    return generator.generate_all_reports(format);
}

FormattedMonthlyReports ReportService::run_export_all_monthly_reports_query(ReportFormat format) const {
    MonthlyReportService generator(db_, app_config_);
    return generator.generate_reports(format);
}

FormattedPeriodReports ReportService::run_export_all_period_reports_query(const std::vector<int>& days_list, ReportFormat format) const {
    PeriodReportService generator(db_, app_config_);
    return generator.generate_reports(days_list, format);
}