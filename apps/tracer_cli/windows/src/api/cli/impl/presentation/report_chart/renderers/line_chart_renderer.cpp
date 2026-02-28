// api/cli/impl/presentation/report_chart/renderers/line_chart_renderer.cpp
#include "api/cli/impl/presentation/report_chart/renderers/line_chart_renderer.hpp"

#include <memory>

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

namespace {

class LineChartRenderer final : public IChartRenderer {
public:
  [[nodiscard]] auto BuildSpec() const -> ChartRendererSpec override {
    return {.kind_token = "line", .title_label = "Line"};
  }
};

} // namespace

auto CreateLineChartRenderer() -> std::unique_ptr<IChartRenderer> {
  return std::make_unique<LineChartRenderer>();
}

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
