// queries/daily/formatters/tex/DayTex.hpp
#ifndef DAILY_REPORT_TEX_FORMATTER_HPP
#define DAILY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp"

// --- Forward Declarations ---
struct DailyReportData;


class DayTex : public IReportFormatter<DailyReportData>, private BaseTexFormatter {
public:
    DayTex() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override;
    
    void format_content(std::stringstream& ss, const struct MonthlyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const struct PeriodReportData& data, sqlite3* db) const override {}

    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    // [新增] 新增的私有方法声明
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_HPP