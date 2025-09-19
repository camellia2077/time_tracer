// queries/daily/formatters/typ/DayTyp.hpp
#ifndef DAY_TYP_HPP
#define DAY_TYP_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"
#include <memory>
#include <sstream>

class DayTyp : public IReportFormatter<DailyReportData> {
public:
    explicit DayTyp(std::shared_ptr<DayTypConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    std::shared_ptr<DayTypConfig> config_;
};

#endif // DAY_TYP_HPP