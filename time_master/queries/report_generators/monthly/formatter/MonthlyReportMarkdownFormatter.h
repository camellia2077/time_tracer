#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_H
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_H

#include "IReportFormatter.h" // [修改] 继承自接口
#include <sstream>

/**
 * @class MonthlyReportMarkdownFormatter
 * @brief 将月报数据格式化为 Markdown 字符串的具体实现。
 */
class MonthlyReportMarkdownFormatter : public IReportFormatter {
public:
    MonthlyReportMarkdownFormatter() = default;

    /**
     * @brief [修改] 重写接口的 format_report 方法。
     */
    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_H