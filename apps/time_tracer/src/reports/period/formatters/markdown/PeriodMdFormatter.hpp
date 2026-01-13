// reports/period/formatters/markdown/PeriodMdFormatter.hpp
#ifndef PERIOD_MD_FORMATTER_HPP
#define PERIOD_MD_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseMdFormatter.hpp"
#include "reports/period/formatters/markdown/PeriodMdConfig.hpp"
#include "reports/shared/model/PeriodReportData.hpp"

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

#endif // PERIOD_MD_FORMATTER_HPP