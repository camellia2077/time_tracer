// reports/period/formatters/markdown/period_md_formatter.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_FORMATTER_HPP_
#define REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_FORMATTER_HPP_

#include "reports/shared/formatters/templates/base_md_formatter.hpp"
#include "reports/period/formatters/markdown/period_md_config.hpp"
#include "reports/data/model/period_report_data.hpp"

namespace reporting {

class PeriodMdFormatter : public BaseMdFormatter<PeriodReportData, PeriodMdConfig> {
public:
    explicit PeriodMdFormatter(std::shared_ptr<PeriodMdConfig> config);

protected:
    std::string validate_data(const PeriodReportData& data) const override;
    bool is_empty_data(const PeriodReportData& data) const override;
    int get_avg_days(const PeriodReportData& data) const override;
    std::string get_no_records_msg() const override;
    void format_header_content(std::stringstream& ss, const PeriodReportData& data) const override;
};

} // namespace reporting

#endif // REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_FORMATTER_HPP_