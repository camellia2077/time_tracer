// infra/insights/range/formatters/latex/range_tex_utils.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_

#include <memory>
#include <string>

#include "domain/insights/models/range_insights_data.hpp"
#include "infra/insights/range/formatters/latex/range_tex_config.hpp"

namespace RangeTexUtils {

void DisplaySummary(std::string& insights_stream,

                    const RangeInsightsData& data,
                    const std::shared_ptr<RangeTexConfig>& config);

}  // namespace RangeTexUtils

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
