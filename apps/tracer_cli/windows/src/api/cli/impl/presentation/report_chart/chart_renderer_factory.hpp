// api/cli/impl/presentation/report_chart/chart_renderer_factory.hpp
#pragma once

#include <memory>

#include "api/cli/impl/presentation/report_chart/chart_renderer.hpp"
#include "api/cli/impl/presentation/report_chart/report_chart_types.hpp"

namespace tracer_core::cli::impl::presentation::report_chart {

auto CreateChartRenderer(ReportChartHtmlType chart_type)
    -> std::unique_ptr<IChartRenderer>;

} // namespace tracer_core::cli::impl::presentation::report_chart
