// queries/export/AllPeriodReports.cpp
#include "AllPeriodReports.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/factories/FormatterFactory.hpp" // [修改] 引入新的统一工厂
#include <stdexcept>
#include <memory>

AllPeriodReports::AllPeriodReports(sqlite3* db, const AppConfig& config) 
    : m_db(db), app_config_(config) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;

    // [核心修改] 使用统一工厂创建格式化器
    auto formatter = FormatterFactory::create_period_formatter(format, app_config_);

    for (int days : days_list) {
        if (days > 0) {
            PeriodQuerier querier(m_db, days);
            PeriodReportData report_data = querier.fetch_data();

            std::string formatted_report = formatter->format_report(report_data);
            reports[days] = formatted_report;
        }
    }

    return reports;
}