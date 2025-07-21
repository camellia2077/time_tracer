// PeriodTex.h
#ifndef PERIOD_REPORT_TEX_FORMATTER_H
#define PERIOD_REPORT_TEX_FORMATTER_H

#include "report_generators/period/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class PeriodTex
 * @brief 将周期报告数据格式化为 TeX 字符串的具体实现。
 * (现在使用 ProjectBreakdownFormatterFactory 来处理项目明细)
 */
class PeriodTex : public IReportFormatter {
public:
    PeriodTex() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    // _escape_tex 函数声明已被移除
    void _display_preamble(std::stringstream& ss) const;
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_H