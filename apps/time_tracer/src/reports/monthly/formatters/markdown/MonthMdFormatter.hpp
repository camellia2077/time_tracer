// reports/monthly/formatters/markdown/MonthMdFormatter.hpp
#ifndef MONTH_MD_FORMATTER_HPP
#define MONTH_MD_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseMdFormatter.hpp"
#include "reports/monthly/formatters/markdown/MonthMdConfig.hpp"
#include "reports/shared/model/MonthlyReportData.hpp"

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

#endif // MONTH_MD_FORMATTER_HPP