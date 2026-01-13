// reports/daily/formatters/latex/DayTexFormatter.hpp
#ifndef DAY_TEX_FORMATTER_HPP
#define DAY_TEX_FORMATTER_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/daily/formatters/latex/DayTexConfig.hpp"
#include <memory>

class DayTexFormatter : public IReportFormatter<DailyReportData> {
public:
    explicit DayTexFormatter(std::shared_ptr<DayTexConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    std::shared_ptr<DayTexConfig> config_;
};

#endif // DAY_TEX_FORMATTER_HPP