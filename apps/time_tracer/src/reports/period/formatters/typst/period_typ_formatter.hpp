// reports/period/formatters/typst/period_typ_formatter.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_FORMATTER_HPP_
#define REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_typ_formatter.hpp"
#include "reports/period/formatters/typst/period_typ_config.hpp"
#include "reports/data/model/period_report_data.hpp"

namespace reporting {

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

} // namespace reporting

#endif // REPORTS_PERIOD_FORMATTERS_TYPST_PERIOD_TYP_FORMATTER_HPP_