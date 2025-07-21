#ifndef DAILY_REPORT_TEX_FORMATTER_H
#define DAILY_REPORT_TEX_FORMATTER_H

#include "report_generators/daily/formatter/IReportFormatter.h"
#include <sstream>

/**
 * @class DailyTex
 * @brief 将日报数据格式化为 TeX 字符串的具体实现。
 */
class DailyTex : public IReportFormatter {
public:
    DailyTex() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) override;

private:
    void _display_preamble(std::stringstream& ss) const;
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    std::string _escape_tex(const std::string& s) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_H