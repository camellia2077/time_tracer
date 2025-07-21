// MonthlyTex.h
#ifndef MONTHLY_REPORT_TEX_FORMATTER_H
#define MONTHLY_REPORT_TEX_FORMATTER_H

#include "report_generators/monthly/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class MonthlyTex
 * @brief 将月报数据格式化为 TeX 字符串的具体实现。
 * (现在使用 ProjectBreakdownFormatterFactory 来处理项目明细)
 */
class MonthlyTex : public IReportFormatter {
public:
    MonthlyTex() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    // _escape_tex 函数声明已被移除
    void _display_preamble(std::stringstream& ss) const;
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_H