// queries/monthly/MonthGenerator.cpp

#include "MonthGenerator.hpp"
#include "MonthQuerier.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"

#include "queries/monthly/formatters/typ/MonthTypConfig.hpp"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"

// [修改] 构造函数实现
MonthGenerator::MonthGenerator(sqlite3* db, const std::string& month_typ_config_path)
    : m_db(db), m_month_typ_config_path(month_typ_config_path) {}

std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    MonthQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();
    std::unique_ptr<IReportFormatter<MonthlyReportData>> formatter;

    switch (format) {
        case ReportFormat::Typ: {
            auto config = std::make_shared<MonthTypConfig>(m_month_typ_config_path);
            formatter = std::make_unique<MonthTyp>(config);
            break;
        }
        case ReportFormat::Markdown: {
            formatter = std::make_unique<MonthMd>();
            break;
        }
        case ReportFormat::LaTeX: {
            formatter = std::make_unique<MonthTex>();
            break;
        }
    }

    return formatter->format_report(report_data, m_db);
}