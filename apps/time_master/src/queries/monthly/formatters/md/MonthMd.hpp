// queries/monthly/formatters/md/MonthMd.hpp
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  
#include "queries/shared/data/MonthlyReportData.hpp"
#include <sstream>

// Forward declarations
struct MonthlyReportData;
struct sqlite3; // Forward-declare the opaque struct sqlite3

/**
 * @class MonthMd
 * @brief Concrete implementation for formatting monthly report data into a Markdown string.
 */
class MonthMd : public IReportFormatter<MonthlyReportData> {
public:
    MonthMd() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP