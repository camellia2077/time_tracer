#ifndef QUERY_PERIOD_H
#define QUERY_PERIOD_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "common_utils.h" // For ProjectTree etc.

/**
 * @class PeriodReportGenerator
 * @brief 负责生成一个时间周期（最近N天）的总结报告。
 *
 * 该类封装了计算日期范围、获取数据、处理数据和显示报告的所有逻辑。
 */
class PeriodReportGenerator {
public:
    // 构造函数，需要数据库连接和查询的天数
    explicit PeriodReportGenerator(sqlite3* db, int days_to_query);

    // 生成并显示完整的周期报告
    void generate_report();

private:
    // 私有辅助函数
    bool _validate_input() const;
    void _fetch_data();
    void _display_summary();
    void _display_project_breakdown();

    // 成员变量
    sqlite3* m_db;
    const int m_days_to_query;
    std::string m_start_date;
    std::string m_end_date;
    long long m_total_duration = 0;
    int m_actual_days = 0;
    std::vector<std::pair<std::string, long long>> m_records;
};

#endif // QUERY_PERIOD_H