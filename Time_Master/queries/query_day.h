#ifndef QUERY_DAY_H
#define QUERY_DAY_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include "common_utils.h" // For ProjectTree struct and time_format_duration function

// 用于存储从 'days' 表中获取的元数据
struct DayMetadata {
    std::string status = "N/A";
    std::string remark = "N/A";
    std::string getup_time = "N/A";
};

/**
 * @class DailyReportGenerator
 * @brief 负责生成单日详细报告的类。
 *
 * 该类封装了获取数据、处理数据和显示报告的所有逻辑。
 */
class DailyReportGenerator {
public:
    // 构造函数，需要数据库连接和查询日期
    explicit DailyReportGenerator(sqlite3* db, const std::string& date);

    // 生成并显示完整的每日报告
    void generate_report();

private:
    // 私有辅助函数，对应拆分后的各项功能
    void _fetch_metadata();
    void _fetch_total_duration();
    void _fetch_time_records();
    void _display_header();
    void _display_project_breakdown();

    // 成员变量，用于存储状态和数据
    sqlite3* m_db;
    const std::string m_date;
    DayMetadata m_metadata;
    long long m_total_duration = 0;
    std::vector<std::pair<std::string, long long>> m_records;
};

#endif // QUERY_DAY_H