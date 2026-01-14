// reports/monthly/formatters/latex/MonthTexUtils.hpp
#ifndef MONTH_TEX_UTILS_HPP
#define MONTH_TEX_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/model/MonthlyReportData.hpp"
#include "reports/monthly/formatters/latex/MonthTexConfig.hpp"

namespace MonthTexUtils {

    /**
     * @brief 为月报生成 LaTeX 格式的摘要（标题和总结信息）。
     */
    void display_summary(std::stringstream& ss, const MonthlyReportData& data, const std::shared_ptr<MonthTexConfig>& config);

} // namespace MonthTexUtils

#endif // MONTH_TEX_UTILS_HPP