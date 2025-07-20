#ifndef DAILY_REPORT_MARKDOWN_FORMATTER_H
#define DAILY_REPORT_MARKDOWN_FORMATTER_H

#include "IReportFormatter.h"
#include <sstream>

/**
 * @class DailyReportMarkdownFormatter
 * @brief 将日报数据格式化为 Markdown 字符串的具体实现。
 */
class DailyReportMarkdownFormatter : public IReportFormatter {
public:
    DailyReportMarkdownFormatter() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_MARKDOWN_FORMATTER_H