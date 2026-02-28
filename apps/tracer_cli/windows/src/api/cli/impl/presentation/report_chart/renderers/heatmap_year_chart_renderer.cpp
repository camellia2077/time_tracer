// api/cli/impl/presentation/report_chart/renderers/heatmap_year_chart_renderer.cpp
#include "api/cli/impl/presentation/report_chart/renderers/heatmap_year_chart_renderer.hpp"

#include <memory>

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

namespace {

class HeatmapYearChartRenderer final : public IChartRenderer {
public:
  [[nodiscard]] auto BuildSpec() const -> ChartRendererSpec override {
    return {.kind_token = "heatmap-year", .title_label = "Heatmap Year"};
  }
};

} // namespace

auto CreateHeatmapYearChartRenderer() -> std::unique_ptr<IChartRenderer> {
  return std::make_unique<HeatmapYearChartRenderer>();
}

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
