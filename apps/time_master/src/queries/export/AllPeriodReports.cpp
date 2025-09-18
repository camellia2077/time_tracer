// queries/export/AllPeriodReports.cpp

#include "AllPeriodReports.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include <stdexcept>
#include <memory>

#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp" 
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"
#include "common/AppConfig.hpp"

AllPeriodReports::AllPeriodReports(sqlite3* db, const AppConfig& config) 
    : m_db(db), app_config_(config) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;

    std::unique_ptr<IReportFormatter<PeriodReportData>> formatter;
    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<PeriodTypConfig>(app_config_.period_typ_config_path);
            formatter = std::make_unique<PeriodTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            auto config = std::make_shared<PeriodMdConfig>(app_config_.period_md_config_path);
            formatter = std::make_unique<PeriodMd>(config);
            break;
        }
        case ReportFormat::LaTeX: {
            auto config = std::make_shared<PeriodTexConfig>(app_config_.period_tex_config_path);
            formatter = std::make_unique<PeriodTex>(config);
            break;
        }
    }

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