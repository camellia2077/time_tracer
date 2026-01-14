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

// [新增] 引入工厂头文件以便进行注册
#include "reports/shared/factories/GenericFormatterFactory.hpp"

// [新增] 静态注册逻辑
// 使用匿名命名空间和立即执行的 lambda 来确保只注册一次
namespace {
    bool register_formatters() {
        // --- 注册日报格式 (DailyReportData) ---
        using DayFactory = GenericFormatterFactory<DailyReportData>;
        DayFactory::register_dll_formatter(ReportFormat::Markdown, "DayMdFormatter");
        DayFactory::register_dll_formatter(ReportFormat::Typ,      "DayTypFormatter");
        DayFactory::register_dll_formatter(ReportFormat::LaTeX,    "DayTexFormatter");

        // --- 注册月报格式 (MonthlyReportData) ---
        using MonthFactory = GenericFormatterFactory<MonthlyReportData>;
        MonthFactory::register_dll_formatter(ReportFormat::Markdown, "MonthMdFormatter");
        MonthFactory::register_dll_formatter(ReportFormat::Typ,      "MonthTypFormatter");
        MonthFactory::register_dll_formatter(ReportFormat::LaTeX,    "MonthTexFormatter");

        // --- 注册周期报告格式 (PeriodReportData) ---
        using PeriodFactory = GenericFormatterFactory<PeriodReportData>;
        PeriodFactory::register_dll_formatter(ReportFormat::Markdown, "PeriodMdFormatter");
        PeriodFactory::register_dll_formatter(ReportFormat::LaTeX,    "PeriodTexFormatter");
        PeriodFactory::register_dll_formatter(ReportFormat::Typ,      "PeriodTypFormatter");

        return true;
    }

    // 静态变量初始化会触发上面的函数执行
    bool is_registered = register_formatters();
}

ReportService::ReportService(sqlite3* db, const AppConfig& config)
    : db_(db), app_config_(config) {}

std::string ReportService::run_daily_query(const std::string& date, ReportFormat format) const {
    BaseGenerator<DailyReportData, DayQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(date, format);
}

std::string ReportService::run_monthly_query(const std::string& month, ReportFormat format) const {
    BaseGenerator<MonthlyReportData, MonthQuerier, const std::string&> generator(db_, app_config_);
    return generator.generate_report(month, format);
}

std::string ReportService::run_period_query(int days, ReportFormat format) const {
    BaseGenerator<PeriodReportData, PeriodQuerier, int> generator(db_, app_config_);
    return generator.generate_report(days, format);
}

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