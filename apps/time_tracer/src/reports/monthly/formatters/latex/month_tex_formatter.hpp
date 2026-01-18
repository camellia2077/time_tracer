// reports/monthly/formatters/latex/month_tex_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_HPP_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_tex_formatter.hpp"
#include "reports/monthly/formatters/latex/month_tex_config.hpp"
#include "reports/shared/model/monthly_report_data.hpp"

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