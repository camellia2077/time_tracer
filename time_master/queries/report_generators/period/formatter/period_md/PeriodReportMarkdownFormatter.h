#ifndef PERIOD_REPORT_MARKDOWN_FORMATTER_H
#define PERIOD_REPORT_MARKDOWN_FORMATTER_H

#include "report_generators/period/formatter/IReportFormatter.h" // [修改] 继承自接口

#include <sstream>

/**
 * @class PeriodReportMarkdownFormatter
 * @brief 将周期报告数据格式化为 Markdown 字符串的具体实现。
 */
class PeriodReportMarkdownFormatter : public IReportFormatter {
public:
    PeriodReportMarkdownFormatter() = default;

    /**
     * @brief [修改] 重写接口的 format_report 方法。
     */
    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_MARKDOWN_FORMATTER_H