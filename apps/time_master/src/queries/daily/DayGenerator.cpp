// queries/daily/DayGenerator.cpp

// queries/report_generators/daily/DayGenerator.cpp
#include "DayGenerator.hpp"
#include "DayQuerier.hpp"
#include "queries/shared/data/query_data_structs.hpp"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/daily/formatters/md/DayMd.hpp"
#include "queries/daily/formatters/tex/DayTex.hpp"
#include "queries/daily/formatters/typ/DayTyp.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"
#include <memory>

DayGenerator::DayGenerator(sqlite3* db, const std::string& day_typ_config_path) 
    : m_db(db), m_day_typ_config_path(day_typ_config_path) {}

std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    if (format == ReportFormat::Typ) {
        // [MODIFIED] 使用成员变量中存储的路径
        auto config = std::make_shared<DayTypConfig>(m_day_typ_config_path);
        DayTyp formatter(config);
        return formatter.format_report(report_data, m_db);
    }

    auto formatter = ReportFmtFactory<DailyReportData, DayMd, DayTex>::create_formatter(format);


    return formatter->format_report(report_data, m_db);
}