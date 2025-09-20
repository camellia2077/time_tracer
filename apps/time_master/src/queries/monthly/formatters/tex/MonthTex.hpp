// queries/monthly/formatters/tex/MonthTex.hpp
#ifndef MONTHLY_REPORT_TEX_FORMATTER_HPP
#define MONTHLY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp"
#include <memory>
#include <sstream>

class MonthTex : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTex(std::shared_ptr<MonthTexConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;
    
    std::shared_ptr<MonthTexConfig> config_;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_HPP