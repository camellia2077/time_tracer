// api/cli/impl/presentation/report_chart/report_chart_types.hpp
#pragma once

#include <string>
#include <vector>

namespace tracer_core::cli::impl::presentation {

enum class ReportChartHtmlType {
  kLine,
  kBar,
  kPie,
  kHeatmapYear,
  kHeatmapMonth,
};

enum class ReportChartTheme {
  kDefault,
  kGithub,
};

struct ReportChartHeatmapVisualConfig {
  std::vector<double> positive_hours_thresholds;
  std::vector<std::string> palette_colors;
};

} // namespace tracer_core::cli::impl::presentation
