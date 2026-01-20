// reports/monthly/formatters/typst/month_typ_formatter.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_HPP_
#define REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_typ_formatter.hpp"
#include "reports/monthly/formatters/typst/month_typ_config.hpp"
#include "reports/data/model/monthly_report_data.hpp"

class MonthTypFormatter : public BaseTypFormatter<MonthlyReportData, MonthTypConfig> {
public:
    explicit MonthTypFormatter(std::shared_ptr<MonthTypConfig> config);

protected:
    std::string validate_data(const MonthlyReportData& data) const override;
    bool is_empty_data(const MonthlyReportData& data) const override;
    int get_avg_days(const MonthlyReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const MonthlyReportData& data) const override;
    void format_page_setup(std::stringstream& ss) const override;
};

#endif // REPORTS_MONTHLY_FORMATTERS_TYPST_MONTH_TYP_FORMATTER_HPP_