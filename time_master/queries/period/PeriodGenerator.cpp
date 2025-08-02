#include "common/pch.h"
#include "queries/period/PeriodGenerator.h"
#include "queries/period/PeriodQuerier.h"
#include "queries/shared/PeriodReportData.h"

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/FmtFactory.h"
#include "queries/period/formatters/md/PeriodMd.h"
#include "queries/period/formatters/tex/PeriodTex.h"
#include "queries/period/formatters/typ/PeriodTyp.h"


PeriodGenerator::PeriodGenerator(sqlite3* db) : m_db(db) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    // [修改] 使用新的模板工厂创建格式化器
    auto formatter = ReportFmtFactory<PeriodReportData, PeriodMd, PeriodTex, PeriodTyp>::create_formatter(format);

    return formatter->format_report(report_data, m_db);
}