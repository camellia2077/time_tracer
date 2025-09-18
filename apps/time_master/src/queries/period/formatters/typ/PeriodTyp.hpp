// queries/period/formatters/typ/PeriodTyp.hpp
#ifndef PERIOD_REPORT_TYP_FORMATTER_HPP
#define PERIOD_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/typ/PeriodTypConfig.hpp"
#include <sstream>
#include <memory>

class PeriodTyp : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTyp(std::shared_ptr<PeriodTypConfig> config); 

    std::string format_report(const PeriodReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const;

    std::shared_ptr<PeriodTypConfig> config_;
};

#endif // PERIOD_REPORT_TYP_FORMATTER_HPP