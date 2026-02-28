// api/cli/impl/presentation/report_chart/renderers/bar_chart_renderer.cpp
#include "api/cli/impl/presentation/report_chart/renderers/bar_chart_renderer.hpp"

#include <memory>

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

namespace {

class BarChartRenderer final : public IChartRenderer {
public:
  [[nodiscard]] auto BuildSpec() const -> ChartRendererSpec override {
    return {.kind_token = "bar", .title_label = "Bar"};
  }
};

} // namespace

auto CreateBarChartRenderer() -> std::unique_ptr<IChartRenderer> {
  return std::make_unique<BarChartRenderer>();
}

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
