#ifndef MONTHLY_REPORT_TEX_FORMATTER_H
#define MONTHLY_REPORT_TEX_FORMATTER_H

#include "report_generators/monthly/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class MonthlyTex
 * @brief 将月报数据格式化为 TeX 字符串的具体实现。
 */
class MonthlyTex : public IReportFormatter {
public:
    MonthlyTex() = default;

    /**
     * @brief 重写接口的 format_report 方法。
     */
    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    // 从 DailyTex 中复用的辅助函数
    void _display_preamble(std::stringstream& ss) const;
    std::string _escape_tex(const std::string& s) const;

    // 月报特有的函数
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_H