// queries/daily/formatters/tex/DayTex.hpp
#ifndef DAILY_REPORT_TEX_FORMATTER_HPP
#define DAILY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp"

// --- Forward Declarations ---
struct DailyReportData;
// [新增] 加上对其他数据结构的前置声明
struct MonthlyReportData;
struct PeriodReportData;


class DayTex : public IReportFormatter<DailyReportData>, private BaseTexFormatter {
public:
    DayTex() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    // 这是 DayTex 真正需要实现的函数
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override;
    
    // [核心修改] 删除错误的无参数版本，并为空实现另外两个纯虚函数
    void format_content(std::stringstream& /*ss*/, const MonthlyReportData& /*data*/, sqlite3* /*db*/) const override {}
    void format_content(std::stringstream& /*ss*/, const PeriodReportData& /*data*/, sqlite3* /*db*/) const override {}

    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_HPP