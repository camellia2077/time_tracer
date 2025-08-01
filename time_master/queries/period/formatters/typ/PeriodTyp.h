// PeriodTyp.h
#ifndef PERIOD_REPORT_TYP_FORMATTER_H
#define PERIOD_REPORT_TYP_FORMATTER_H

#include "queries/shared/Interface/IReportFormatter.h"
#include "queries/shared/PeriodReportData.h"
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

#endif // PERIOD_REPORT_TYP_FORMATTER_H