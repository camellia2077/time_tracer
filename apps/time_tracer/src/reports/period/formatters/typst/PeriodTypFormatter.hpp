// reports/period/formatters/typst/PeriodTypFormatter.hpp
#ifndef PERIOD_TYP_FORMATTER_HPP
#define PERIOD_TYP_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseTypFormatter.hpp"
#include "reports/period/formatters/typst/PeriodTypConfig.hpp"
#include "reports/shared/model/PeriodReportData.hpp"

class PeriodTypFormatter : public BaseTypFormatter<PeriodReportData, PeriodTypConfig> {
public:
    explicit PeriodTypFormatter(std::shared_ptr<PeriodTypConfig> config);

protected:
    std::string validate_data(const PeriodReportData& data) const override;
    bool is_empty_data(const PeriodReportData& data) const override;
    int get_avg_days(const PeriodReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const PeriodReportData& data) const override;
    void format_page_setup(std::stringstream& ss) const override;
};

#endif // PERIOD_TYP_FORMATTER_HPP