// queries/report_generators/monthly/MonthGenerator.cpp
#include "MonthGenerator.h"

#include "MonthQuerier.h"
#include "queries/shared/MonthlyReportData.h"

#include "queries/monthly/MonthFmtFactory.h" //  引入格式化器工厂，替代具体的格式化器

MonthGenerator::MonthGenerator(sqlite3* db) : m_db(db) {}

// 更新函数签名以接收 ReportFormat 参数
std::string MonthGenerator::generate_report(const std::string& year_month, ReportFormat format) {
    // 1. 使用 Querier 模块获取数据 (此部分逻辑不变)
    MonthQuerier querier(m_db, year_month);
    MonthlyReportData report_data = querier.fetch_data();

    // 2. 使用工厂根据指定格式创建格式化器实例
    auto formatter = MonthFmtFactory::create_formatter(format);

    // 3. 使用创建好的格式化器接口来生成报告字符串
    return formatter->format_report(report_data, m_db);
}