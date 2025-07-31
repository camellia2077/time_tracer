// queries/daily/formatters/DayMd.h
#ifndef DAILY_REPORT_MARKDOWN_FORMATTER_H
#define DAILY_REPORT_MARKDOWN_FORMATTER_H

#include "queries/daily/IDayFmt.h"
#include <sstream>

// Forward declaration
struct DailyReportData;

/**
 * @class DayMd
 * @brief 将日报数据格式化为 Markdown 字符串的具体实现。
 */
class DayMd : public IDayFmt {
public:
    DayMd() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_MARKDOWN_FORMATTER_H