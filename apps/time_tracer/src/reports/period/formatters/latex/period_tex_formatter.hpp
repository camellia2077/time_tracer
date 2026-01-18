// reports/period/formatters/latex/period_tex_formatter.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_FORMATTER_HPP_
#define REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_tex_formatter.hpp"
#include "reports/period/formatters/latex/period_tex_config.hpp"
#include "reports/shared/model/period_report_data.hpp"

namespace reporting {

class PeriodTexFormatter : public BaseTexFormatter<PeriodReportData, PeriodTexConfig> {
public:
    explicit PeriodTexFormatter(std::shared_ptr<PeriodTexConfig> config);

protected:
    std::string validate_data(const PeriodReportData& data) const override;
    bool is_empty_data(const PeriodReportData& data) const override;
    int get_avg_days(const PeriodReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const PeriodReportData& data) const override;
};

} // namespace reporting

#endif // REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_FORMATTER_HPP_