// queries/monthly/MonthGenerator.cpp

#include "MonthGenerator.hpp"
#include "MonthQuerier.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/monthly/formatters/md/MonthMd.hpp"
#include "queries/monthly/formatters/tex/MonthTex.hpp"
#include "queries/monthly/formatters/typ/MonthTyp.hpp"

MonthGenerator::MonthGenerator(sqlite3* db) : m_db(db) {}

std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    MonthQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();

    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<MonthlyReportData, MonthMd, MonthTex, MonthTyp>::create_formatter(format);

    return formatter->format_report(report_data, m_db);
}