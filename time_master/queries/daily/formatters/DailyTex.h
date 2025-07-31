// queries/daily/formatters/DailyTex.h
#ifndef DAILY_REPORT_TEX_FORMATTER_H
#define DAILY_REPORT_TEX_FORMATTER_H

#include "queries/daily/IDailyReportFormatter.h"
#include "queries/shared/BaseTexFormatter.h" 

// --- Forward Declarations ---
struct MonthlyReportData;
struct PeriodReportData;
struct DailyReportData;


class DailyTex : public IDailyReportFormatter, private BaseTexFormatter {
public:
    DailyTex() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void format_content(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const override;
    
    // -- Empty implementations to satisfy the base class's pure virtual functions --
    void format_content(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const override {}
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override {}

    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_TEX_FORMATTER_H