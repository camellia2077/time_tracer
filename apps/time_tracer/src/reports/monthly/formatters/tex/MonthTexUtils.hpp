// reports/monthly/formatters/tex/MonthTexUtils.hpp
#ifndef MONTH_TEX_UTILS_HPP
#define MONTH_TEX_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/data/MonthlyReportData.hpp"
#include "reports/monthly/formatters/tex/MonthTexConfig.hpp"

namespace MonthTexUtils {

    /**
     * @brief 为月报生成 LaTeX 格式的摘要（标题和总结信息）。
     * @param ss stringstream 用于写入输出。
     * @param data 包含报告数据的 MonthlyReportData 对象。
     * @param config 指向 MonthTexConfig 的共享指针。
     */
    void display_summary(std::stringstream& ss, const MonthlyReportData& data, const std::shared_ptr<MonthTexConfig>& config);

} // namespace MonthTexUtils

#endif // MONTH_TEX_UTILS_HPP