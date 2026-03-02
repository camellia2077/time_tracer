// api/cli/impl/presentation/report_chart/chart_renderer_factory.cpp
#include "api/cli/impl/presentation/report_chart/chart_renderer_factory.hpp"

#include "api/cli/impl/presentation/report_chart/renderers/bar_chart_renderer.hpp"
#include "api/cli/impl/presentation/report_chart/renderers/heatmap_month_chart_renderer.hpp"
#include "api/cli/impl/presentation/report_chart/renderers/heatmap_year_chart_renderer.hpp"
#include "api/cli/impl/presentation/report_chart/renderers/line_chart_renderer.hpp"
#include "api/cli/impl/presentation/report_chart/renderers/pie_chart_renderer.hpp"

namespace tracer_core::cli::impl::presentation::report_chart {

auto CreateChartRenderer(ReportChartHtmlType chart_type)
    -> std::unique_ptr<IChartRenderer> {
  switch (chart_type) {
  case ReportChartHtmlType::kBar:
    return renderers::CreateBarChartRenderer();
  case ReportChartHtmlType::kHeatmapYear:
    return renderers::CreateHeatmapYearChartRenderer();
  case ReportChartHtmlType::kHeatmapMonth:
    return renderers::CreateHeatmapMonthChartRenderer();
  case ReportChartHtmlType::kPie:
    return renderers::CreatePieChartRenderer();
  case ReportChartHtmlType::kLine:
  default:
    return renderers::CreateLineChartRenderer();
  }
}

} // namespace tracer_core::cli::impl::presentation::report_chart
