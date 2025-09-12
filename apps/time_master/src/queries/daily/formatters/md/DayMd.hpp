// queries/daily/formatters/DayMd.hpp
#ifndef DAILY_REPORT_MARKDOWN_FORMATTER_HPP
#define DAILY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp" // 引入新的模板接口
#include "queries/shared/data/DailyReportData.hpp"   // 引入数据类型
#include <sstream>

// Forward declaration
struct DailyReportData;

/**
 * @class DayMd
 * @brief 将日报数据格式化为 Markdown 字符串的具体实现。
 */
// 继承自模板化的通用接口
class DayMd : public IReportFormatter<DailyReportData> {
public:
    DayMd() = default;

    // 函数签名与模板接口完全匹配，无需更改
    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const; //获取活动细节
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const; // [新增] 显示统计信息
};

#endif // DAILY_REPORT_MARKDOWN_FORMATTER_HPP