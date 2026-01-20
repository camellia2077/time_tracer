// reports/monthly/formatters/markdown/month_md_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_HPP_
#define REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_md_formatter.hpp"
#include "reports/monthly/formatters/markdown/month_md_config.hpp"
#include "reports/data/model/monthly_report_data.hpp"

class MonthMdFormatter : public BaseMdFormatter<MonthlyReportData, MonthMdConfig> {
public:
    explicit MonthMdFormatter(std::shared_ptr<MonthMdConfig> config);

protected:
    std::string validate_data(const MonthlyReportData& data) const override;
    bool is_empty_data(const MonthlyReportData& data) const override;
    int get_avg_days(const MonthlyReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const MonthlyReportData& data) const override;
};

#endif // REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_FORMATTER_HPP_