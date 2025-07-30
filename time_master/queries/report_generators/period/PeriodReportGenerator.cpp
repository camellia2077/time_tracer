#include "PeriodReportGenerator.h"
#include "querier/PeriodReportQuerier.h"
#include "queries/report_generators/period/_period_data/PeriodReportData.h"
// [修改] 引入格式化器工厂，替代具体的格式化器
#include "formatter/PeriodReportFormatterFactory.h"

PeriodReportGenerator::PeriodReportGenerator(sqlite3* db) : m_db(db) {}

// [修改] 更新函数签名以接收 ReportFormat 参数
std::string PeriodReportGenerator::generate_report(int days, ReportFormat format) {
    // 1. 使用 Querier 获取数据 (此部分逻辑不变)
    PeriodReportQuerier querier(m_db, days);
    PeriodReportData report_data = querier.fetch_data();

    // 2. [修改] 使用工厂根据指定格式创建格式化器实例
    auto formatter = PeriodReportFormatterFactory::create_formatter(format);

    // 3. [修改] 使用创建好的格式化器接口来生成报告字符串
    return formatter->format_report(report_data, m_db);
}