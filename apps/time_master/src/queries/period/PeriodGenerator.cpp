// queries/period/PeriodGenerator.cpp

#include "queries/period/PeriodGenerator.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include <memory>

#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"

PeriodGenerator::PeriodGenerator(sqlite3* db, const std::string& period_typ_config_path, const std::string& period_md_config_path)
    : m_db(db), m_period_typ_config_path(period_typ_config_path), m_period_md_config_path(period_md_config_path) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();
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

    return formatter->format_report(report_data, m_db);
}