// MonthlyTex.h (corrected)
#ifndef MONTHLY_REPORT_TEX_FORMATTER_H
#define MONTHLY_REPORT_TEX_FORMATTER_H

#include "queries/monthly/IMonthFmt.h" // Corrected: Was IPeriodFmt.h
#include "queries/shared/BaseTexFormatter.h" 

// --- Forward Declarations ---
struct DailyReportData;
struct PeriodReportData;
struct MonthlyReportData;

/**
 * @class MonthlyTex
 * @brief 将月报数据格式化为 TeX 字符串的具体实现。
 * 通过继承 BaseTexFormatter 来复用文档框架。
 */
// Corrected: Inherits from the correct interface
class MonthlyTex : public IMonthFmt, private BaseTexFormatter { 
public:
    MonthlyTex() = default;

    // This override is now correct because the base class is correct
    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    // Implements the pure virtual "hook" function from the base class
    void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const override;

    // -- Empty implementations to satisfy base class requirements --
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override {}

    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_H