
// queries/report_generators/daily/DayGenerator.cpp
#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/data/query_data_structs.hpp"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/daily/formatters/md/DayMd.hpp"
#include "queries/daily/formatters/tex/DayTex.hpp"
#include "queries/daily/formatters/typ/DayTyp.hpp"

DayGenerator::DayGenerator(sqlite3* db) : m_db(db) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<DailyReportData, DayMd, DayTex, DayTyp>::create_formatter(format);

    return formatter->format_report(report_data, m_db);
}