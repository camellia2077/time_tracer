// queries/monthly/formatters/MonthMd.h
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_H
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_H

#include "queries/monthly/IMonthFmt.h" // Corrected: Was IPeriodFmt.h
#include <sstream>

// Forward declaration
struct MonthlyReportData;

/**
 * @class MonthMd
 * @brief 将月报数据格式化为 Markdown 字符串的具体实现。
 * (现在使用 ProjectBreakdownFormatterFactory 来处理项目明细)
 */
class MonthMd : public IMonthFmt { // Corrected: Was IReportFormatter
public:
    MonthMd() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_H