#include "common/pch.h"
// queries/report_generators/daily/DayGenerator.cpp
#include "DayGenerator.h"
#include "DayQuerier.h"
#include "queries/shared/query_data_structs.h"
#include "queries/daily/DayFmtFactory.h" // Correct include

DayGenerator::DayGenerator(sqlite3* db) : m_db(db) {}

// [修改] 函数签名与头文件保持一致
std::string DayGenerator::generate_report(const std::string& date, ReportFormat format) {
    // 使用 Querier 模块获取数据 (此部分逻辑不变)
    DayQuerier querier(m_db, date);
    DailyReportData report_data = querier.fetch_data();

    // 使用工厂根据指定格式创建格式化器实例
    // 这里不再直接实例化 DailyReportFormatter，而是通过工厂获取一个实现了
    // IReportFormatter 接口的对象。
    auto formatter = DayFmtFactory::create_formatter(format);

    // 使用创建好的格式化器来生成报告字符串
    // 调用的是接口的 format_report 方法，具体执行哪个实现取决于工厂返回的对象。
    return formatter->format_report(report_data, m_db);
}