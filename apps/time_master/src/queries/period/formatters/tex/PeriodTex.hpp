// queries/period/formatters/tex/PeriodTex.hpp
#ifndef PERIOD_TEX_HPP
#define PERIOD_TEX_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"  
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"
#include <memory>
#include <sstream>

class PeriodTex : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTex(std::shared_ptr<PeriodTexConfig> config);
    std::string format_report(const PeriodReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const;

    std::shared_ptr<PeriodTexConfig> config_;
};

#endif // PERIOD_TEX_HPP