// queries/report_generators/daily/formatter/day_md/DailyMarkdown.h
#ifndef DAILY_REPORT_MARKDOWN_FORMATTER_H
#define DAILY_REPORT_MARKDOWN_FORMATTER_H

#include "queries/report_generators/daily/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class DailyMarkdown
 * @brief 将日报数据格式化为 Markdown 字符串的具体实现。
 */
class DailyMarkdown : public IReportFormatter {
public:
    DailyMarkdown() = default;

    // The 'const' keyword has been removed to match the base class interface
    std::string format_report(const DailyReportData& data, sqlite3* db) override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_MARKDOWN_FORMATTER_H