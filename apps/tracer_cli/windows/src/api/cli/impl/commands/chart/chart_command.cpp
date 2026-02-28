// api/cli/impl/commands/chart/chart_command.cpp
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/commands/query/heatmap_config.hpp"
#include "api/cli/impl/presentation/report_chart_html_exporter.hpp"
#include "api/cli/impl/utils/tree_formatter.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "shared/types/exceptions.hpp"

class ChartCommand : public ICommand {
public:
  explicit ChartCommand(ITracerCoreApi &core_api) : core_api_(core_api) {}

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "chart", ICommand::GetCategory());
  }
  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
};

namespace {
namespace fs = std::filesystem;

using tracer_core::cli::impl::presentation::ReportChartHeatmapVisualConfig;
using tracer_core::cli::impl::presentation::ReportChartHtmlType;
using tracer_core::cli::impl::presentation::ReportChartTheme;
using tracer_core::core::dto::DataQueryAction;
using tracer_core::core::dto::DataQueryOutputMode;
using tracer_core::core::dto::DataQueryRequest;

[[nodiscard]] auto ParseChartType(std::string_view value)
    -> std::optional<ReportChartHtmlType> {
  if (value == "line") {
    return ReportChartHtmlType::kLine;
  }
  if (value == "bar") {
    return ReportChartHtmlType::kBar;
  }
  if (value == "pie") {
    return ReportChartHtmlType::kPie;
  }
  if (value == "heatmap-year") {
    return ReportChartHtmlType::kHeatmapYear;
  }
  if (value == "heatmap-month") {
    return ReportChartHtmlType::kHeatmapMonth;
  }
  return std::nullopt;
}

[[nodiscard]] auto ParseChartTheme(std::string_view value)
    -> std::optional<ReportChartTheme> {
  if (value == "default") {
    return ReportChartTheme::kDefault;
  }
  if (value == "github") {
    return ReportChartTheme::kGithub;
  }
  return std::nullopt;
}

[[nodiscard]] auto IsHeatmapChartType(ReportChartHtmlType chart_type) -> bool {
  return chart_type == ReportChartHtmlType::kHeatmapYear ||
         chart_type == ReportChartHtmlType::kHeatmapMonth;
}

[[nodiscard]] auto ChartTypeToken(ReportChartHtmlType chart_type)
    -> std::string_view {
  switch (chart_type) {
  case ReportChartHtmlType::kBar:
    return "bar";
  case ReportChartHtmlType::kPie:
    return "pie";
  case ReportChartHtmlType::kHeatmapYear:
    return "heatmap-year";
  case ReportChartHtmlType::kHeatmapMonth:
    return "heatmap-month";
  case ReportChartHtmlType::kLine:
  default:
    return "line";
  }
}

void EnsureTextOutputSuccess(const tracer_core::core::dto::TextOutput &response,
                             std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  if (!response.error_message.empty()) {
    throw tracer_core::common::LogicError(response.error_message);
  }
  throw tracer_core::common::LogicError(std::string(fallback_message));
}

auto BuildReportChartRequest(const ParsedArgs &args) -> DataQueryRequest {
  DataQueryRequest request;
  request.action = DataQueryAction::kReportChart;
  request.output_mode = DataQueryOutputMode::kSemanticJson;

  if (args.Has("root")) {
    request.root = args.Get("root");
  }
  if (args.Has("year")) {
    request.year = args.GetAsInt("year");
  }
  if (args.Has("month")) {
    request.month = args.GetAsInt("month");
  }
  if (args.Has("from")) {
    request.from_date = tracer_core::cli::impl::utils::NormalizeDateInput(
        args.Get("from"), false);
  }
  if (args.Has("to")) {
    request.to_date =
        tracer_core::cli::impl::utils::NormalizeDateInput(args.Get("to"), true);
  }
  if (args.Has("lookback_days")) {
    const int lookback_days = args.GetAsInt("lookback_days");
    if (lookback_days <= 0) {
      throw std::runtime_error("--lookback-days must be greater than 0.");
    }
    request.lookback_days = lookback_days;
  }

  return request;
}

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterChartCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "chart", [](AppContext &ctx) -> std::unique_ptr<ChartCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        return std::make_unique<ChartCommand>(*ctx.core_api);
      });
}

} // namespace tracer_core::cli::impl::commands

auto ChartCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"output",
       ArgType::kOption,
       {"-o", "--output"},
       "Output HTML path",
       false,
       ""},
      {"type",
       ArgType::kOption,
       {"--type"},
       "Chart type (line, bar, pie, heatmap-year, heatmap-month)",
       false,
       "line"},
      {"theme",
       ArgType::kOption,
       {"--theme"},
       "Chart theme (default, github)",
       false,
       "default"},
      {"heatmap_palette",
       ArgType::kOption,
       {"--heatmap-palette"},
       "Heatmap palette name from config/charts/heatmap.toml",
       false,
       ""},
      {"list_heatmap_palettes",
       ArgType::kFlag,
       {"--list-heatmap-palettes"},
       "List heatmap palette names from config/charts/heatmap.toml and exit",
       false,
       ""},
      {"root",
       ArgType::kOption,
       {"--root"},
       "Root filter (exact root or root_*)",
       false,
       ""},
      {"year", ArgType::kOption, {"--year"}, "Filter by year", false, ""},
      {"month", ArgType::kOption, {"--month"}, "Filter by month", false, ""},
      {"from",
       ArgType::kOption,
       {"--from"},
       "Filter start date (YYYY, YYYYMM, YYYYMMDD)",
       false,
       ""},
      {"to",
       ArgType::kOption,
       {"--to"},
       "Filter end date (YYYY, YYYYMM, YYYYMMDD)",
       false,
       ""},
      {"lookback_days",
       ArgType::kOption,
       {"--lookback-days"},
       "Lookback window in days",
       false,
       ""},
  };
}

auto ChartCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "chart", ICommand::GetHelp());
}

void ChartCommand::Execute(const CommandParser &parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const bool list_heatmap_palettes = args.Has("list_heatmap_palettes");
  if (list_heatmap_palettes) {
    if (args.Has("heatmap_palette") || args.Has("root") || args.Has("year") ||
        args.Has("month") || args.Has("from") || args.Has("to") ||
        args.Has("lookback_days")) {
      throw std::runtime_error(
          "--list-heatmap-palettes does not support chart rendering filters.");
    }

    const fs::path heatmap_config_path = tracer_core::cli::impl::commands::
        query::heatmap::ResolveHeatmapConfigPath(fs::path(parser.GetRawArg(0)));
    const auto heatmap_config =
        tracer_core::cli::impl::commands::query::heatmap::LoadHeatmapConfig(
            heatmap_config_path);
    const auto palette_names =
        tracer_core::cli::impl::commands::query::heatmap::ListPaletteNames(
            heatmap_config);

    std::cout << "Heatmap palettes (" << heatmap_config_path.string() << "):\n";
    for (const auto &palette_name : palette_names) {
      std::cout << "- " << palette_name;
      if (palette_name == heatmap_config.default_light_palette) {
        std::cout << " (default-light)";
      }
      if (palette_name == heatmap_config.default_dark_palette) {
        std::cout << " (default-dark)";
      }
      std::cout << '\n';
    }
    return;
  }

  const std::optional<ReportChartHtmlType> chart_type =
      ParseChartType(args.Get("type"));
  if (!chart_type.has_value()) {
    throw std::runtime_error(
        "--type must be one of: line, bar, pie, heatmap-year, heatmap-month.");
  }
  const std::optional<ReportChartTheme> chart_theme =
      ParseChartTheme(args.Get("theme"));
  if (!chart_theme.has_value()) {
    throw std::runtime_error("--theme must be one of: default, github.");
  }

  if (!args.Has("output")) {
    throw std::runtime_error("--output is required.");
  }

  const std::string requested_output = args.Get("output");
  if (requested_output.empty()) {
    throw std::runtime_error("--output must not be empty.");
  }
  const fs::path output_path = fs::absolute(fs::path(requested_output));

  std::optional<std::string> chart_heatmap_palette_name;
  if (args.Has("heatmap_palette")) {
    const std::string palette_name = args.Get("heatmap_palette");
    if (palette_name.empty()) {
      throw std::runtime_error("--heatmap-palette must not be empty.");
    }
    chart_heatmap_palette_name = palette_name;
  }

  if (chart_heatmap_palette_name.has_value() &&
      !IsHeatmapChartType(chart_type.value())) {
    throw std::runtime_error(
        "--heatmap-palette only supports heatmap-year or heatmap-month chart "
        "type.");
  }

  ReportChartHeatmapVisualConfig heatmap_visual_config{};
  if (IsHeatmapChartType(chart_type.value())) {
    const fs::path heatmap_config_path = tracer_core::cli::impl::commands::
        query::heatmap::ResolveHeatmapConfigPath(fs::path(parser.GetRawArg(0)));
    const auto heatmap_config =
        tracer_core::cli::impl::commands::query::heatmap::LoadHeatmapConfig(
            heatmap_config_path);
    const std::string resolved_palette_name =
        chart_heatmap_palette_name.value_or(
            heatmap_config.default_light_palette);
    heatmap_visual_config.positive_hours_thresholds =
        heatmap_config.positive_hours;
    heatmap_visual_config.palette_colors =
        tracer_core::cli::impl::commands::query::heatmap::ResolvePaletteColors(
            heatmap_config, resolved_palette_name);
  }

  const DataQueryRequest request = BuildReportChartRequest(args);
  const auto response = core_api_.RunDataQuery(request);
  EnsureTextOutputSuccess(response, "Chart query failed.");

  tracer_core::cli::impl::presentation::ExportReportChartHtml(
      response.content, output_path, chart_type.value(), chart_theme.value(),
      heatmap_visual_config);

  std::cout << "Saved chart " << ChartTypeToken(chart_type.value())
            << " HTML: " << output_path.string() << '\n';
}
