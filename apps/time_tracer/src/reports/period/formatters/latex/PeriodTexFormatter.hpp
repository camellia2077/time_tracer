// reports/period/formatters/latex/PeriodTexFormatter.hpp
#ifndef PERIOD_TEX_FORMATTER_HPP
#define PERIOD_TEX_FORMATTER_HPP

#include "reports/shared/formatters/templates/BaseTexFormatter.hpp"
#include "reports/period/formatters/latex/PeriodTexConfig.hpp"
#include "reports/shared/model/PeriodReportData.hpp"

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

#endif // PERIOD_TEX_FORMATTER_HPP