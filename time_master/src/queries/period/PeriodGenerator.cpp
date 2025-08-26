
#include "queries/period/PeriodGenerator.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/data/PeriodReportData.hpp"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"


PeriodGenerator::PeriodGenerator(sqlite3* db) : m_db(db) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<PeriodReportData, PeriodMd, PeriodTex, PeriodTyp>::create_formatter(format);

    return formatter->format_report(report_data, m_db);
}