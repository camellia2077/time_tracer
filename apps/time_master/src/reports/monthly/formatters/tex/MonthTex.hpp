// reports/monthly/formatters/tex/MonthTex.hpp
#ifndef MONTH_TEX_HPP
#define MONTH_TEX_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/monthly/formatters/tex/MonthTexConfig.hpp"
#include <memory>
#include <sstream>

class MonthTex : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTex(std::shared_ptr<MonthTexConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    // [修改] 私有的 display 方法已被移除，逻辑已移至 MonthTexUtils
    std::shared_ptr<MonthTexConfig> config_;
};

#endif // MONTH_TEX_HPP