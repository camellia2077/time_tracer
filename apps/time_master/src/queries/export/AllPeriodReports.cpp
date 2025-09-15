// queries/export/AllPeriodReports.cpp

#include "AllPeriodReports.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include <stdexcept>
#include <memory>

#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"

AllPeriodReports::AllPeriodReports(sqlite3* db, const std::string& period_typ_config_path, const std::string& period_md_config_path) 
    : m_db(db), m_period_typ_config_path(period_typ_config_path), m_period_md_config_path(period_md_config_path) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list, ReportFormat format) {
    FormattedPeriodReports reports;

    std::unique_ptr<IReportFormatter<PeriodReportData>> formatter;
    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<PeriodTypConfig>(m_period_typ_config_path);
            formatter = std::make_unique<PeriodTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            auto config = std::make_shared<PeriodMdConfig>(m_period_md_config_path);
            formatter = std::make_unique<PeriodMd>(config);
            break;
        }
        case ReportFormat::LaTeX: {
            formatter = std::make_unique<PeriodTex>();
            break;
        }
    }

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