// queries/period/formatters/typ/PeriodTyp.hpp
#ifndef PERIOD_REPORT_TYP_FORMATTER_HPP
#define PERIOD_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp" // [新增]
#include <sstream>
#include <memory> // [新增]

class PeriodTyp : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTyp(std::shared_ptr<PeriodTypConfig> config); // [修改]

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;

    std::shared_ptr<PeriodTypConfig> config_; // [新增]
};

#endif // PERIOD_REPORT_TYP_FORMATTER_HPP