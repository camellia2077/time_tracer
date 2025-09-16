// queries/monthly/MonthGenerator.cpp

#include "MonthGenerator.hpp"
#include "MonthQuerier.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "common/AppConfig.hpp"

#include "queries/monthly/formatters/typ/MonthTypConfig.hpp"
#include "queries/monthly/formatters/md/MonthMdConfig.hpp"
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"
#include <memory>

MonthGenerator::MonthGenerator(sqlite3* db, const AppConfig& config)
    : m_db(db), app_config_(config) {}

std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    MonthQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();
    std::unique_ptr<IReportFormatter<MonthlyReportData>> formatter;

    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<MonthTypConfig>(app_config_.month_typ_config_path);
            formatter = std::make_unique<MonthTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            auto config = std::make_shared<MonthMdConfig>(app_config_.month_md_config_path);
            formatter = std::make_unique<MonthMd>(config);
            break;
        }
        case ReportFormat::LaTeX: {
            auto config = std::make_shared<MonthTexConfig>(app_config_.month_tex_config_path);
            formatter = std::make_unique<MonthTex>(config);
            break;
        }
    }

    return formatter->format_report(report_data, m_db);
}