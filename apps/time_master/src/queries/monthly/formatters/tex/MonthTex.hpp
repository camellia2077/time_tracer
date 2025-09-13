// queries/monthly/formatters/tex/MonthTex.hpp
#ifndef MONTHLY_REPORT_TEX_FORMATTER_HPP
#define MONTHLY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp"

// --- Forward Declarations ---
struct DailyReportData;
struct PeriodReportData;
struct MonthlyReportData;
struct sqlite3; // Forward-declare sqlite3

/**
 * @class MonthTex
 * @brief Concrete implementation to format monthly report data into a TeX string.
 * Reuses the document frame by inheriting from BaseTexFormatter.
 */
class MonthTex : public IReportFormatter<MonthlyReportData>, private BaseTexFormatter {
    public:
        MonthTex() = default;
    
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

#endif // MONTHLY_REPORT_TEX_FORMATTER_HPP