// api/cli/impl/presentation/report_chart/renderers/bar_chart_renderer.hpp
#pragma once

#include <memory>

#include "api/cli/impl/presentation/report_chart/chart_renderer.hpp"

namespace tracer_core::cli::impl::presentation::report_chart::renderers {

auto CreateBarChartRenderer() -> std::unique_ptr<IChartRenderer>;

} // namespace tracer_core::cli::impl::presentation::report_chart::renderers
