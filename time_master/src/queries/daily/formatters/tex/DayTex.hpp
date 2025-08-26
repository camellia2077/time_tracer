// queries/daily/formatters/DayTex.hpp
#ifndef DAILY_REPORT_TEX_FORMATTER_HPP
#define DAILY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  // Replaced IDayFmt.hpp
#include "queries/shared/data/DailyReportData.hpp"    // Added for the template type
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp" 

// --- Forward Declarations ---
struct DailyReportData;


class DayTex : public IReportFormatter<DailyReportData>, private BaseTexFormatter {
public:
    DayTex() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    // This is the primary content formatter for DayTex
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override;
    
    // -- Empty implementations to satisfy the base class's pure virtual functions --
    void format_content(std::stringstream& ss, const struct MonthlyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const struct PeriodReportData& data, sqlite3* db) const override {}

    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_HPP