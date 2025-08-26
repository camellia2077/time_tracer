// queries/monthly/formatters/typ/MonthTyp.hpp
#ifndef MONTHLY_REPORT_TYP_FORMATTER_HPP
#define MONTHLY_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include <sstream>

// Forward declaration
struct MonthlyReportData;

class MonthTyp : public IReportFormatter<MonthlyReportData> {
public:
    MonthTyp() = default;

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
};

#endif // MONTHLY_REPORT_TYP_FORMATTER_HPP