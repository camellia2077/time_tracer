// PeriodMarkdown.h
#ifndef PERIOD_REPORT_MARKDOWN_FORMATTER_H
#define PERIOD_REPORT_MARKDOWN_FORMATTER_H

#include "queries/period/IPeriodFmt.h" // This path is corrected
#include <sstream>

// Forward declaration
struct PeriodReportData;

/**
 * @class PeriodMarkdown
 * @brief 将周期报告数据格式化为 Markdown 字符串的具体实现。
 * (现在使用 ProjectBreakdownFormatterFactory 来处理项目明细)
 */
class PeriodMarkdown : public IPeriodFmt { // Inherits from the correct class
public:
    PeriodMarkdown() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_MARKDOWN_FORMATTER_H