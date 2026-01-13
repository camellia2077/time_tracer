// reports/daily/formatters/typst/DayTyp.hpp
#ifndef DAY_TYP_HPP
#define DAY_TYP_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "reports/daily/formatters/typst/DayTypConfig.hpp"
#include <memory>

#include <sstream>

class DayTypFormatter : public IReportFormatter<DailyReportData> {
public:
    explicit DayTypFormatter(std::shared_ptr<DayTypConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    std::shared_ptr<DayTypConfig> config_;
};

#endif // DAY_TYP_HPP