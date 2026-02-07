// infrastructure/reports/range/formatters/latex/range_tex_utils.hpp
#ifndef REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
#define REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_

#include <memory>
#include <sstream>

#include "domain/reports/models/range_report_data.hpp"
#include "infrastructure/reports/range/formatters/latex/range_tex_config.hpp"

namespace RangeTexUtils {

void DisplaySummary(std::stringstream& report_stream,

                    const RangeReportData& data,
                    const std::shared_ptr<RangeTexConfig>& config);

}  // namespace RangeTexUtils

#endif  // REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
