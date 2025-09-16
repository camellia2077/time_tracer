#include "queries/period/PeriodGenerator.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include <memory>

#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"

PeriodGenerator::PeriodGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();
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

    return formatter->format_report(report_data, m_db);
}