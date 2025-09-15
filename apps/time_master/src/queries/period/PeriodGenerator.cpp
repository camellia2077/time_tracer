// queries/period/PeriodGenerator.cpp

#include "queries/period/PeriodGenerator.hpp"
#include "queries/period/PeriodQuerier.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include <memory>

// [修改] 引入新的通用工厂和具体的格式化器类
#include "queries/shared/factories/FmtFactory.hpp"
#include "queries/period/formatters/md/PeriodMd.hpp"
#include "queries/period/formatters/tex/PeriodTex.hpp"
#include "queries/period/formatters/typ/PeriodTyp.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp" // [新增] 引入周期报告Typst配置类

// [修改] 构造函数，接收数据库连接和周期报告Typst配置路径
PeriodGenerator::PeriodGenerator(sqlite3* db, const std::string& period_typ_config_path)
    : m_db(db), m_period_typ_config_path(period_typ_config_path) {}

std::string PeriodGenerator::generate_report(int days, ReportFormat format) {
    PeriodQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    if (format == ReportFormat::Typ) {
        // [修改] 如果格式为Typst，则使用配置路径来创建专门的格式化器
        auto config = std::make_shared<PeriodTypConfig>(m_period_typ_config_path);
        PeriodTyp formatter(config);
        return formatter.format_report(report_data, m_db);
    }

    // 对于其他格式，继续使用通用工厂
    auto formatter = ReportFmtFactory<PeriodReportData, PeriodMd, PeriodTex>::create_formatter(format);

    return formatter->format_report(report_data, m_db);
}