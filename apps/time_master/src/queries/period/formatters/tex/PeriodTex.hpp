// queries/period/formatters/tex/PeriodTex.hpp
#ifndef PERIOD_REPORT_TEX_FORMATTER_HPP
#define PERIOD_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/shared/formatters/tex/BaseTexFormatter.hpp" 

// --- Forward Declarations ---
struct DailyReportData;
struct MonthlyReportData;
struct PeriodReportData;
struct sqlite3; // Forward-declare the opaque struct sqlite3

/**
 * @class PeriodTex
 * @brief Concrete implementation for formatting period report data into a TeX string.
 */
class PeriodTex : public IReportFormatter<PeriodReportData>, private BaseTexFormatter {
public:
    PeriodTex() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    // Implements the pure virtual "hook" function from the base class
    void format_content(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const override;

    // -- Empty implementations to satisfy base class requirements --
    void format_content(std::stringstream& /*ss*/, const DailyReportData& /*data*/, sqlite3* /*db*/) const override {}
    void format_content(std::stringstream& /*ss*/, const MonthlyReportData& /*data*/, sqlite3* /*db*/) const override {}

    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_HPP