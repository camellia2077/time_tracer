// queries/daily/DayGenerator.cpp
#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/data/query_data_structs.hpp"

#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/daily/formatters/md/DayMd.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include "queries/daily/formatters/tex/DayTex.hpp"
#include "queries/daily/formatters/tex/DayTexConfig.hpp" // [新增] 引入新的TeX配置类
#include "queries/daily/formatters/typ/DayTyp.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"
#include <memory>

DayGenerator::DayGenerator(sqlite3* db, const std::string& day_typ_config_path, const std::string& day_md_config_path, const std::string& day_tex_config_path)
    : m_db(db), m_day_typ_config_path(day_typ_config_path), m_day_md_config_path(day_md_config_path), m_day_tex_config_path(day_tex_config_path) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    std::unique_ptr<IReportFormatter<DailyReportData>> formatter;

    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<DayTypConfig>(m_day_typ_config_path);
            formatter = std::make_unique<DayTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            auto config = std::make_shared<DayMdConfig>(m_day_md_config_path);
            formatter = std::make_unique<DayMd>(config);
            break;
        }
        case ReportFormat::LaTeX: {
            // [修改] 使用新的配置类来实例化 DayTex
            auto config = std::make_shared<DayTexConfig>(m_day_tex_config_path);
            formatter = std::make_unique<DayTex>(config);
            break;
        }
    }

    return formatter->format_report(report_data, m_db);
}