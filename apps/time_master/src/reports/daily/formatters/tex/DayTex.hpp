// reports/daily/formatters/tex/DayTex.hpp
#ifndef DAY_TEX_HPP
#define DAY_TEX_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include <memory>

class DayTex : public IReportFormatter<DailyReportData> {
public:
    explicit DayTex(std::shared_ptr<DayTexConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    // [修改] 所有私有 _display_* 方法都将被移除
    std::shared_ptr<DayTexConfig> config_;
};

#endif // DAY_TEX_HPP