// queries/export/AllPeriodReports.cpp

#include "AllPeriodReports.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include <stdexcept>

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"

AllPeriodReports::AllPeriodReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;
    
    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<PeriodReportData, PeriodMd, PeriodTex, PeriodTyp>::create_formatter(format);

    for (int days : days_list) {
        if (days > 0) {
            PeriodQuerier querier(m_db, days);
            PeriodReportData report_data = querier.fetch_data();

            std::string formatted_report = formatter->format_report(report_data, m_db);
            reports[days] = formatted_report;
        }
    }

    return reports;
}