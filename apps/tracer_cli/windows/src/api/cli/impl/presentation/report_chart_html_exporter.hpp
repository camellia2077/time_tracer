// api/cli/impl/presentation/report_chart_html_exporter.hpp
#pragma once

#include <filesystem>
#include <string_view>

#include "api/cli/impl/presentation/report_chart/report_chart_types.hpp"

namespace tracer_core::cli::impl::presentation {

auto ExportReportChartHtml(
    std::string_view semantic_payload, const std::filesystem::path &output_path,
    ReportChartHtmlType chart_type, ReportChartTheme chart_theme,
    const ReportChartHeatmapVisualConfig &heatmap_visual_config = {}) -> void;

auto ExportReportChartLineHtml(std::string_view semantic_payload,
                               const std::filesystem::path &output_path)
    -> void;

} // namespace tracer_core::cli::impl::presentation
