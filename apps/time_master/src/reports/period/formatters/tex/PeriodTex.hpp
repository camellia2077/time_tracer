// reports/period/formatters/tex/PeriodTex.hpp
#ifndef PERIOD_TEX_HPP
#define PERIOD_TEX_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"  
#include "reports/shared/data/PeriodReportData.hpp"
#include "reports/period/formatters/tex/PeriodTexConfig.hpp"
#include <memory>
#include <sstream>

class PeriodTex : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTex(std::shared_ptr<PeriodTexConfig> config);
    std::string format_report(const PeriodReportData& data) const override;

private:
    // [修改] 私有的 display 方法已被移除，逻辑已移至 PeriodTexUtils
    std::shared_ptr<PeriodTexConfig> config_;
};

#endif // PERIOD_TEX_HPP