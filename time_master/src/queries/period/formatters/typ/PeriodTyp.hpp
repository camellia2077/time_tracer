// PeriodTyp.hpp
#ifndef PERIOD_REPORT_TYP_FORMATTER_HPP
#define PERIOD_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include <sstream>

// Forward declaration
struct PeriodReportData;

class PeriodTyp : public IReportFormatter<PeriodReportData> {
public:
    PeriodTyp() = default;

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;
};

#endif // PERIOD_REPORT_TYP_FORMATTER_HPP