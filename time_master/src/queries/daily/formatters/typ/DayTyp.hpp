// queries/daily/formatters/typ/DayTyp.hpp
#ifndef DAILY_REPORT_TYP_FORMATTER_H
#define DAILY_REPORT_TYP_FORMATTER_H

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"

#include <sstream>

// Forward declaration
struct DailyReportData;

class DayTyp : public IReportFormatter<DailyReportData> {
public:
    DayTyp() = default;

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
};

#endif // DAILY_REPORT_TYP_FORMATTER_H