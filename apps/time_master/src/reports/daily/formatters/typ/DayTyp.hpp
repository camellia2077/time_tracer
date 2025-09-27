// reports/daily/formatters/typ/DayTyp.hpp
#ifndef DAY_TYP_HPP
#define DAY_TYP_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/typ/DayTypConfig.hpp"
#include <memory>
#include <sstream>

class DayTyp : public IReportFormatter<DailyReportData> {
public:
    explicit DayTyp(std::shared_ptr<DayTypConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    std::shared_ptr<DayTypConfig> config_;
    // [核心修改] _display_statistics 方法已被移除
};

#endif // DAY_TYP_HPP