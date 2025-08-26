// PeriodTex.hpp
#ifndef PERIOD_REPORT_TEX_FORMATTER_HPP
#define PERIOD_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  
#include "queries/shared/data/PeriodReportData.hpp"  // 为模板类型引入定义
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp" 

// --- 前置声明 ---
struct DailyReportData;
struct MonthlyReportData;
struct PeriodReportData;

/**
 * @class PeriodTex
 * @brief 将周期报告数据格式化为 TeX 字符串的具体实现。
 */
class PeriodTex : public IReportFormatter<PeriodReportData>, private BaseTexFormatter {
public:
    PeriodTex() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    // 实现基类定义的纯虚“钩子”函数
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override;

    // -- 为满足基类的纯虚函数要求，提供空的实现 --
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const override {}

    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_H