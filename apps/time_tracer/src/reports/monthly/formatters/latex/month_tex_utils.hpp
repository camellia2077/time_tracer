// reports/monthly/formatters/latex/month_tex_utils.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_
#define REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_

#include <memory>
#include <sstream>

#include "reports/data/model/monthly_report_data.hpp"
#include "reports/monthly/formatters/latex/month_tex_config.hpp"

namespace MonthTexUtils {

/**
 * @brief 为月报生成 LaTeX 格式的摘要（标题和总结信息）。
 */
void display_summary(std::stringstream& ss, const MonthlyReportData& data,
                     const std::shared_ptr<MonthTexConfig>& config);

}  // namespace MonthTexUtils

#endif  // REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_