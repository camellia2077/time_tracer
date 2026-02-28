// api/cli/impl/presentation/report_chart/renderers/heatmap_month_chart_renderer.cpp
#include "api/cli/impl/presentation/report_chart/renderers/heatmap_month_chart_renderer.hpp"

#include <memory>

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

namespace {

class HeatmapMonthChartRenderer final : public IChartRenderer {
public:
  [[nodiscard]] auto BuildSpec() const -> ChartRendererSpec override {
    return {.kind_token = "heatmap-month", .title_label = "Heatmap Month"};
  }
};

} // namespace

auto CreateHeatmapMonthChartRenderer() -> std::unique_ptr<IChartRenderer> {
  return std::make_unique<HeatmapMonthChartRenderer>();
}

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
