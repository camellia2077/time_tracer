#ifndef PERIOD_REPORT_TEX_FORMATTER_H
#define PERIOD_REPORT_TEX_FORMATTER_H

#include "report_generators/period/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class PeriodTex
 * @brief 将周期报告数据格式化为 TeX 字符串的具体实现。
 */
class PeriodTex : public IReportFormatter {
public:
    PeriodTex() = default;

    /**
     * @brief 重写接口的 format_report 方法。
     */
    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    // 从 MonthlyTex 中复用的辅助函数
    void _display_preamble(std::stringstream& ss) const;
    std::string _escape_tex(const std::string& s) const;

    // 周期报告特有的函数
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_H