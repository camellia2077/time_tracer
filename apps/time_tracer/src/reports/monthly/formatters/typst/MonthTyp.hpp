// reports/monthly/formatters/typst/MonthTyp.hpp
#ifndef MONTH_TYP_HPP
#define MONTH_TYP_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/model/MonthlyReportData.hpp"
#include "reports/monthly/formatters/typst/MonthTypConfig.hpp"
#include <sstream>
#include <memory>

class MonthTyp : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTyp(std::shared_ptr<MonthTypConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    std::shared_ptr<MonthTypConfig> config_;
};

#endif // MONTH_TYP_HPP