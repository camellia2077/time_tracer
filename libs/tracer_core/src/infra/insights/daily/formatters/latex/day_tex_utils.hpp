// infra/insights/daily/formatters/latex/day_tex_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_

#include <memory>
#include <string>

#include "domain/insights/models/daily_insights_data.hpp"
#include "infra/insights/daily/formatters/latex/day_tex_config.hpp"

namespace DayTexUtils {

void DisplayHeader(std::string& insights_stream, const DailyInsightsData& data,
                   const std::shared_ptr<DayTexConfig>& config);

void DisplayDetailedActivities(std::string& insights_stream,
                               const DailyInsightsData& data,
                               const std::shared_ptr<DayTexConfig>& config);

}  // namespace DayTexUtils

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_LATEX_DAY_TEX_UTILS_H_
