// api/cli/impl/presentation/report_chart/renderers/line_chart_renderer.hpp
#pragma once

#include <memory>

#include "api/cli/impl/presentation/report_chart/chart_renderer.hpp"

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

auto CreateLineChartRenderer() -> std::unique_ptr<IChartRenderer>;

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
