// reports/range/formatters/latex/range_tex_utils.hpp
#ifndef REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
#define REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_

#include <memory>
#include <sstream>

#include "reports/data/model/range_report_data.hpp"
#include "reports/range/formatters/latex/range_tex_config.hpp"

namespace RangeTexUtils {

void display_summary(std::stringstream& report_stream,
                     const RangeReportData& data,
                     const std::shared_ptr<RangeTexConfig>& config);

}  // namespace RangeTexUtils

#endif  // REPORTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_UTILS_H_
