// reports/monthly/formatters/typst/MonthTypFormatter.hpp
#ifndef MONTH_TYP_FORMATTER_HPP
#define MONTH_TYP_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseTypFormatter.hpp"
#include "reports/monthly/formatters/typst/MonthTypConfig.hpp"
#include "reports/shared/model/MonthlyReportData.hpp"

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

#endif // MONTH_TYP_FORMATTER_HPP