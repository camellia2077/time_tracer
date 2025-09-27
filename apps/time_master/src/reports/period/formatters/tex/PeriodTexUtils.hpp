// reports/period/formatters/tex/PeriodTexUtils.hpp
#ifndef PERIOD_TEX_UTILS_HPP
#define PERIOD_TEX_UTILS_HPP

#include <sstream>
#include <memory>
#include "reports/shared/data/PeriodReportData.hpp"
#include "reports/period/formatters/tex/PeriodTexConfig.hpp"

namespace PeriodTexUtils {

    /**
     * @brief 为周期报告生成 LaTeX 格式的摘要（标题和总结信息）。
     * @param ss stringstream 用于写入输出。
     * @param data 包含报告数据的 PeriodReportData 对象。
     * @param config 指向 PeriodTexConfig 的共享指针。
     */
    void display_summary(std::stringstream& ss, const PeriodReportData& data, const std::shared_ptr<PeriodTexConfig>& config);

} // namespace PeriodTexUtils

#endif // PERIOD_TEX_UTILS_HPP