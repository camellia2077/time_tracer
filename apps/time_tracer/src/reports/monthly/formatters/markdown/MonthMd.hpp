// reports/monthly/formatters/markdown/MonthMd.hpp
#ifndef MONTH_MD_HPP
#define MONTH_MD_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"  
#include "reports/shared/model/MonthlyReportData.hpp"
#include "reports/monthly/formatters/markdown/MonthMdConfig.hpp"
#include <sstream>
#include <memory>

class MonthMd : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthMd(std::shared_ptr<MonthMdConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    std::shared_ptr<MonthMdConfig> config_;
};

#endif // MONTH_MD_HPP