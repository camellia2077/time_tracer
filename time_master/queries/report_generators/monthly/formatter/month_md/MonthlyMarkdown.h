// MonthlyMarkdown.h
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_H
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_H

#include "report_generators/monthly/formatter/IReportFormatter.h" // 继承自接口
#include <sstream>

/**
 * @class MonthlyMarkdown
 * @brief 将月报数据格式化为 Markdown 字符串的具体实现。
 * (现在使用 ProjectBreakdownFormatterFactory 来处理项目明细)
 */
class MonthlyMarkdown : public IReportFormatter {
public:
    MonthlyMarkdown() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_H