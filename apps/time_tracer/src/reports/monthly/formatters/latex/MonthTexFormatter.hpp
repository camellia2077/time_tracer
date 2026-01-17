// reports/monthly/formatters/latex/MonthTexFormatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_HPP_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_HPP_

#include "reports/shared/formatters/templates/BaseTexFormatter.hpp"
#include "reports/monthly/formatters/latex/MonthTexConfig.hpp"
#include "reports/shared/model/MonthlyReportData.hpp"

// [修复] 还原正确的类声明
class MonthTexFormatter : public BaseTexFormatter<MonthlyReportData, MonthTexConfig> {
public:
    explicit MonthTexFormatter(std::shared_ptr<MonthTexConfig> config);

protected:
    std::string validate_data(const MonthlyReportData& data) const override;
    bool is_empty_data(const MonthlyReportData& data) const override;
    int get_avg_days(const MonthlyReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const MonthlyReportData& data) const override;
};

#endif // REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_HPP_