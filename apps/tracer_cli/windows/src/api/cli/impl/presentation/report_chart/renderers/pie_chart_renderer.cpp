// api/cli/impl/presentation/report_chart/renderers/pie_chart_renderer.cpp
#include "api/cli/impl/presentation/report_chart/renderers/pie_chart_renderer.hpp"

#include <memory>

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

namespace {

class PieChartRenderer final : public IChartRenderer {
public:
  [[nodiscard]] auto BuildSpec() const -> ChartRendererSpec override {
    return {.kind_token = "pie", .title_label = "Pie"};
  }
};

} // namespace

auto CreatePieChartRenderer() -> std::unique_ptr<IChartRenderer> {
  return std::make_unique<PieChartRenderer>();
}

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
