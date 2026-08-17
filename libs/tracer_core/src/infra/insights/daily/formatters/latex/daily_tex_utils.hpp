// infra/insights/daily/formatters/latex/daily_tex_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_UTILS_H_

#include <memory>
#include <string>

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/latex/daily_tex_formatter_config.hpp"

namespace DailyTexUtils {

void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DailyTexFormatterConfig>& config);

void DisplayDetailedActivities(
    std::string& insights_stream, const DailyInsightsData& data,
    const std::shared_ptr<DailyTexFormatterConfig>& config);

}  // namespace DailyTexUtils

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAILY_TEX_UTILS_H_
