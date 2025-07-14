#include "AllPeriodReports.h"
#include "period/querier/PeriodReportQuerier.h"
#include "period/formatter/PeriodReportFormatter.h"
#include <stdexcept>

AllPeriodReports::AllPeriodReports(sqlite3* db) : m_db(db) {
    if (m_db == nullptr) {
        throw std::invalid_argument("Database connection cannot be null.");
    }
}

/**
 * @brief 生成所有周期报告的实现。
 *
 * 遍历天数列表，为每个有效的天数（大于0）：
 * 1. 使用 PeriodReportQuerier 获取数据。
 * 2. 使用 PeriodReportFormatter 格式化报告。
 * 3. 将结果存入 map 中。
 */
FormattedPeriodReports AllPeriodReports::generate_reports(const std::vector<int>& days_list) {
    FormattedPeriodReports reports;
    PeriodReportFormatter formatter; // 复用格式化器

    for (int days : days_list) {
        if (days > 0) {
            // 步骤 1: 获取数据
            PeriodReportQuerier querier(m_db, days);
            PeriodReportData report_data = querier.fetch_data();

            // 步骤 2: 格式化报告
            std::string formatted_report = formatter.format_report(report_data, m_db);

            // 步骤 3: 存储结果
            reports[days] = formatted_report;
        }
    }

    return reports;
}