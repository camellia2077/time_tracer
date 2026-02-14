// infrastructure/reports/monthly/formatters/latex/month_tex_utils.hpp
#ifndef INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_
#define INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_

#include <memory>
#include <string>

#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/reports/monthly/formatters/latex/month_tex_config.hpp"

namespace MonthTexUtils {

/**
 * @brief 为月报生成 LaTeX
 * 格式的摘要（标题和总结信息）。

 */
void DisplayHeader(std::string& report_stream, const MonthlyReportData& data,
                   const std::shared_ptr<MonthTexConfig>& config);

}  // namespace MonthTexUtils

#endif  // INFRASTRUCTURE_REPORTS_MONTHLY_FORMATTERS_LATEX_MONTH_TEX_UTILS_H_
