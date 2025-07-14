#include "AllMonthlyReports.h"
#include "monthly/formatter/MonthlyReportFormatter.h"
#include "monthly/querier/MonthlyReportQuerier.h"
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdexcept>

/**
 * @brief AllMonthlyReports 类的构造函数。
 * * @param db 指向 SQLite 数据库连接的指针。
 * @throws std::invalid_argument 如果数据库连接为空。
 */
AllMonthlyReports::AllMonthlyReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

/**
 * @brief 生成所有月度报告。
 *
 * 该函数会查询数据库中所有存在记录的年月组合，然后为每个月：
 * 1. 使用 MonthlyReportQuerier 获取该月的详细数据。
 * 2. 使用 MonthlyReportFormatter 将数据格式化成报告字符串。
 * 3. 将结果存入一个按年、月分类的嵌套 map 中。
 * * @return FormattedMonthlyReports 一个包含所有格式化后月报的嵌套 map。
 */
FormattedMonthlyReports AllMonthlyReports::generate_reports() {
    FormattedMonthlyReports reports;
    sqlite3_stmt* stmt;

    // SQL 查询：从 'days' 表中选取所有不同的 'year' 和 'month' 组合，并按时间排序
    const char* sql = "SELECT DISTINCT year, month FROM days ORDER BY year, month;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement to fetch unique year/month pairs.");
    }

    // 创建一个格式化器实例，以在循环中复用
    MonthlyReportFormatter formatter;

    // 遍历数据库中所有唯一的年月组合
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int year = sqlite3_column_int(stmt, 0);
        int month = sqlite3_column_int(stmt, 1);

        // 构造 "YYYYMM" 格式的字符串以传递给月报查询器
        std::stringstream year_month_ss;
        year_month_ss << year << std::setw(2) << std::setfill('0') << month;
        std::string year_month_str = year_month_ss.str();

        // 步骤 1: 复用 MonthlyReportQuerier 获取当月数据
        MonthlyReportQuerier querier(m_db, year_month_str);
        MonthlyReportData data = querier.fetch_data();

        // 如果这个月有实际的记录（总时长大于0），才进行格式化和存储
        if (data.total_duration > 0) {
            // 步骤 2: 复用 MonthlyReportFormatter 格式化报告
            std::string formatted_report = formatter.format_report(data, m_db);

            // 步骤 3: 将格式化后的报告存入结果 map
            reports[year][month] = formatted_report;
        }
    }

    // 释放语句句柄
    sqlite3_finalize(stmt);
    
    return reports;
}
