#include "DailyReportGenerator.h"

/**
 * @brief 引入日报查询器模块 (DailyReportQuerier)
 *
 * 目的:
 * 这个头文件定义了 `DailyReportQuerier` 类。此类专门负责与数据库进行交互，
 * 以获取生成日报所需的所有原始数据。
 *
 * 引入的具体内容:
 * 1. `DailyReportQuerier` 类的完整定义。
 * 2. `fetch_data()` 成员函数的声明，该函数执行实际的数据库查询操作，
 * 并返回一个包含所有数据的 `DailyReportData` 结构体。
 *
 * 在本文件中，我们通过实例化 `DailyReportQuerier` 并调用其 `fetch_data()` 方法，
 * 将数据获取的复杂逻辑委托给它处理。
 */
#include "querier/DailyReportQuerier.h"
/**
 * @brief 引入日报格式化器模块 (DailyReportFormatter)
 *
 * 目的:
 * 这个头文件定义了 `DailyReportFormatter` 类。此类专门负责将从查询器
 * 获取的原始 `DailyReportData` 结构体转换成一段人类可读的、格式化好的字符串。
 *
 * 引入的具体内容:
 * 1. `DailyReportFormatter` 类的完整定义。
 * 2. `format_report()` 成员函数的声明，该函数接收 `DailyReportData`
 * 作为输入，并输出最终的报告字符串。
 *
 * 在本文件中，我们实例化 `DailyReportFormatter` 并调用其 `format_report()` 方法，
 * 将数据格式化的任务委托给它处理。
 */
#include "formatter/DailyReportFormatter.h"
#include "report_generators/_shared/query_data_structs.h" // 引入日查询报告所需的结构体 DailyReportData

DailyReportGenerator::DailyReportGenerator(sqlite3* db) : m_db(db) {}

std::string DailyReportGenerator::generate_report(const std::string& date) {
    // 1. Use the Querier module to fetch data
    DailyReportQuerier querier(m_db, date);
    // 使用在 query_data_structs.h 中定义的 DailyReportData 结构体，来存储查询到的日报数据
    DailyReportData report_data = querier.fetch_data();

    // 2. Use the Formatter module to create the report string
    DailyReportFormatter formatter;
    return formatter.format_report(report_data, m_db);
}