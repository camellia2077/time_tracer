// api/cli/impl/presentation/report_chart/chart_renderer.hpp
#pragma once

#include <string_view>

namespace tracer_core::cli::impl::presentation::report_chart {

struct ChartRendererSpec {
  std::string_view kind_token;
  std::string_view title_label;
};

class IChartRenderer {
public:
  virtual ~IChartRenderer() = default;
  [[nodiscard]] virtual auto BuildSpec() const -> ChartRendererSpec = 0;
};

} // namespace tracer_core::cli::impl::presentation::report_chart
