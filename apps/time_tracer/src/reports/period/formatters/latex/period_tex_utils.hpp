// reports/period/formatters/latex/period_tex_utils.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_UTILS_HPP_
#define REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_UTILS_HPP_

#include <sstream>
#include <memory>
#include "reports/data/model/period_report_data.hpp"
#include "reports/period/formatters/latex/period_tex_config.hpp"

namespace PeriodTexUtils {

    /**
     * @brief 为周期报告生成 LaTeX 格式的摘要（标题和总结信息）。
     * Updated to use reporting::PeriodTexConfig
     */
    void display_summary(std::stringstream& ss, const PeriodReportData& data, const std::shared_ptr<reporting::PeriodTexConfig>& config);

} // namespace PeriodTexUtils

#endif // REPORTS_PERIOD_FORMATTERS_LATEX_PERIOD_TEX_UTILS_HPP_