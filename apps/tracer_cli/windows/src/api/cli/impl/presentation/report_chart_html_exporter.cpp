// api/cli/impl/presentation/report_chart_html_exporter.cpp
#include "api/cli/impl/presentation/report_chart_html_exporter.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <nlohmann/json.hpp>

#include "api/cli/impl/presentation/report_chart/chart_renderer_factory.hpp"
#include "shared/types/exceptions.hpp"

namespace tracer_core::cli::impl::presentation {

namespace {

namespace fs = std::filesystem;
using nlohmann::json;

struct ChartExporterAssets {
  std::string html_base_template;
  std::string line_option_script;
  std::string bar_option_script;
  std::string pie_option_script;
  std::string heatmap_year_option_script;
  std::string heatmap_month_option_script;
  std::string echarts_script;
};

constexpr std::string_view kAssetsDirName = "assets";
constexpr std::string_view kBaseTemplateRelativePath =
    "templates/report_chart_base.html.tpl";
constexpr std::string_view kLineOptionTemplateRelativePath =
    "templates/report_chart_option_line.js.tpl";
constexpr std::string_view kBarOptionTemplateRelativePath =
    "templates/report_chart_option_bar.js.tpl";
constexpr std::string_view kPieOptionTemplateRelativePath =
    "templates/report_chart_option_pie.js.tpl";
constexpr std::string_view kHeatmapYearOptionTemplateRelativePath =
    "templates/report_chart_option_heatmap_year.js.tpl";
constexpr std::string_view kHeatmapMonthOptionTemplateRelativePath =
    "templates/report_chart_option_heatmap_month.js.tpl";
constexpr std::string_view kEChartsRelativePath =
    "vendor/echarts/echarts.min.js";

[[nodiscard]] auto EscapeHtml(std::string_view input) -> std::string {
  std::string out;
  out.reserve(input.size());
  for (const char ch : input) {
    switch (ch) {
    case '&':
      out += "&amp;";
      break;
    case '<':
      out += "&lt;";
      break;
    case '>':
      out += "&gt;";
      break;
    case '"':
      out += "&quot;";
      break;
    case '\'':
      out += "&#39;";
      break;
    default:
      out.push_back(ch);
      break;
    }
  }
  return out;
}

[[nodiscard]] auto IsCaseInsensitiveTokenMatch(std::string_view input,
                                               std::size_t offset,
                                               std::string_view token) -> bool {
  if (offset + token.size() > input.size()) {
    return false;
  }
  for (std::size_t index = 0; index < token.size(); ++index) {
    const auto lhs = static_cast<unsigned char>(input[offset + index]);
    const auto rhs = static_cast<unsigned char>(token[index]);
    if (std::tolower(lhs) != std::tolower(rhs)) {
      return false;
    }
  }
  return true;
}

[[nodiscard]] auto EscapeInlineJsonScript(std::string raw_json) -> std::string {
  std::string out;
  out.reserve(raw_json.size());
  for (std::size_t index = 0; index < raw_json.size(); ++index) {
    if (raw_json[index] == '<' && index + 1 < raw_json.size() &&
        raw_json[index + 1] == '/') {
      out += "<\\/";
      ++index;
      continue;
    }
    out.push_back(raw_json[index]);
  }
  return out;
}

[[nodiscard]] auto EscapeInlineScript(std::string raw_script) -> std::string {
  std::string out;
  out.reserve(raw_script.size());
  for (std::size_t index = 0; index < raw_script.size(); ++index) {
    if (raw_script[index] == '<' && index + 8 <= raw_script.size() &&
        raw_script[index + 1] == '/' &&
        IsCaseInsensitiveTokenMatch(raw_script, index + 2, "script")) {
      out += "<\\/script";
      index += 7;
      continue;
    }
    out.push_back(raw_script[index]);
  }
  return out;
}

[[nodiscard]] auto BuildChartRangeLabel(const json &payload) -> std::string {
  if (payload.contains("from_date") && payload["from_date"].is_string() &&
      payload.contains("to_date") && payload["to_date"].is_string()) {
    return payload["from_date"].get<std::string>() + " ~ " +
           payload["to_date"].get<std::string>();
  }
  if (payload.contains("lookback_days") &&
      payload["lookback_days"].is_number_integer()) {
    return "lookback " + std::to_string(payload["lookback_days"].get<int>()) +
           " days";
  }
  return "unspecified";
}

[[nodiscard]] auto ParseReportChartPayloadForHtml(std::string_view content)
    -> json {
  json payload;
  try {
    payload = json::parse(content);
  } catch (const std::exception &error) {
    throw tracer_core::common::LogicError(
        std::string("Invalid report-chart JSON payload for HTML export: ") +
        error.what());
  }

  if (!payload.is_object()) {
    throw tracer_core::common::LogicError(
        "report-chart payload must be a JSON object for HTML export.");
  }

  if (payload.contains("action") && payload["action"].is_string()) {
    const std::string action = payload["action"].get<std::string>();
    if (action != "report_chart" && action != "report-chart") {
      throw tracer_core::common::LogicError(
          "HTML export only supports report_chart payloads.");
    }
  }

  if (!payload.contains("series") || !payload["series"].is_array()) {
    throw tracer_core::common::LogicError(
        "report-chart payload missing array field `series`.");
  }

  return payload;
}

[[nodiscard]] auto StripUtf8Bom(std::string text) -> std::string {
  if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
      static_cast<unsigned char>(text[1]) == 0xBB &&
      static_cast<unsigned char>(text[2]) == 0xBF) {
    text.erase(0, 3);
  }
  return text;
}

[[nodiscard]] auto ReadTextFile(const fs::path &path, std::string_view label)
    -> std::string {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw tracer_core::common::IoError("Failed to open " + std::string(label) +
                                       ": " + path.string());
  }

  std::ostringstream stream;
  stream << file.rdbuf();
  if (file.bad()) {
    throw tracer_core::common::IoError("Failed to read " + std::string(label) +
                                       ": " + path.string());
  }

  return StripUtf8Bom(stream.str());
}

[[nodiscard]] auto GetExecutableDirectory() -> fs::path {
#ifdef _WIN32
  std::vector<wchar_t> buffer(static_cast<std::size_t>(MAX_PATH));
  for (;;) {
    const DWORD copied = GetModuleFileNameW(nullptr, buffer.data(),
                                            static_cast<DWORD>(buffer.size()));
    if (copied == 0) {
      throw tracer_core::common::IoError(
          "Failed to resolve executable path for chart assets.");
    }
    if (copied < buffer.size() - 1) {
      return fs::path(std::wstring(buffer.data(), copied)).parent_path();
    }
    buffer.resize(buffer.size() * 2);
    if (buffer.size() > static_cast<std::size_t>(32768)) {
      throw tracer_core::common::IoError(
          "Executable path is too long while resolving chart assets.");
    }
  }
#else
  std::error_code ec;
  const fs::path cwd = fs::current_path(ec);
  if (ec) {
    throw tracer_core::common::IoError(
        "Failed to resolve current directory for chart assets.");
  }
  return cwd;
#endif
}

[[nodiscard]] auto ResolveAssetsRoot() -> fs::path {
  const std::array<fs::path, 7> required_paths{
      fs::path(kBaseTemplateRelativePath),
      fs::path(kLineOptionTemplateRelativePath),
      fs::path(kBarOptionTemplateRelativePath),
      fs::path(kPieOptionTemplateRelativePath),
      fs::path(kHeatmapYearOptionTemplateRelativePath),
      fs::path(kHeatmapMonthOptionTemplateRelativePath),
      fs::path(kEChartsRelativePath),
  };

  std::vector<fs::path> candidates;
  candidates.push_back(GetExecutableDirectory() / fs::path(kAssetsDirName));

  std::error_code ec;
  const fs::path cwd = fs::current_path(ec);
  if (!ec) {
    candidates.push_back(cwd / fs::path(kAssetsDirName));
  }

  for (const auto &candidate : candidates) {
    const bool has_all_required =
        std::all_of(required_paths.begin(), required_paths.end(),
                    [&candidate](const fs::path &relative_path) {
                      return fs::exists(candidate / relative_path);
                    });
    if (has_all_required) {
      return candidate;
    }
  }

  std::ostringstream message;
  message << "Unable to locate chart assets directory. Expected files:";
  for (const auto &relative_path : required_paths) {
    message << " " << relative_path.string();
  }
  message << ". Checked paths:";
  for (const auto &candidate : candidates) {
    message << " " << candidate.string();
  }
  throw tracer_core::common::IoError(message.str());
}

[[nodiscard]] auto LoadChartExporterAssets() -> const ChartExporterAssets & {
  static const ChartExporterAssets assets = [] {
    const fs::path assets_root = ResolveAssetsRoot();
    ChartExporterAssets loaded;
    loaded.html_base_template =
        ReadTextFile(assets_root / fs::path(kBaseTemplateRelativePath),
                     "chart base HTML template");
    loaded.line_option_script =
        ReadTextFile(assets_root / fs::path(kLineOptionTemplateRelativePath),
                     "line chart option template");
    loaded.bar_option_script =
        ReadTextFile(assets_root / fs::path(kBarOptionTemplateRelativePath),
                     "bar chart option template");
    loaded.pie_option_script =
        ReadTextFile(assets_root / fs::path(kPieOptionTemplateRelativePath),
                     "pie chart option template");
    loaded.heatmap_year_option_script = ReadTextFile(
        assets_root / fs::path(kHeatmapYearOptionTemplateRelativePath),
        "heatmap-year chart option template");
    loaded.heatmap_month_option_script = ReadTextFile(
        assets_root / fs::path(kHeatmapMonthOptionTemplateRelativePath),
        "heatmap-month chart option template");
    loaded.echarts_script =
        ReadTextFile(assets_root / fs::path(kEChartsRelativePath),
                     "vendored ECharts script");
    return loaded;
  }();
  return assets;
}

[[nodiscard]] auto ResolveChartOptionScript(const ChartExporterAssets &assets,
                                            ReportChartHtmlType chart_type)
    -> const std::string & {
  switch (chart_type) {
  case ReportChartHtmlType::kBar:
    return assets.bar_option_script;
  case ReportChartHtmlType::kPie:
    return assets.pie_option_script;
  case ReportChartHtmlType::kHeatmapYear:
    return assets.heatmap_year_option_script;
  case ReportChartHtmlType::kHeatmapMonth:
    return assets.heatmap_month_option_script;
  case ReportChartHtmlType::kLine:
  default:
    return assets.line_option_script;
  }
}

auto ReplaceTemplateToken(std::string &text, std::string_view token,
                          std::string_view replacement) -> void {
  std::size_t offset = 0;
  bool replaced = false;
  while (true) {
    const std::size_t pos = text.find(token, offset);
    if (pos == std::string::npos) {
      break;
    }
    text.replace(pos, token.size(), replacement);
    offset = pos + replacement.size();
    replaced = true;
  }
  if (!replaced) {
    throw tracer_core::common::LogicError(
        "Chart HTML template missing token: " + std::string(token));
  }
}

[[nodiscard]] auto
BuildResolvedHeatmapVisualConfig(const ReportChartHeatmapVisualConfig &input,
                                 ReportChartTheme chart_theme)
    -> ReportChartHeatmapVisualConfig {
  ReportChartHeatmapVisualConfig resolved = input;

  const bool has_valid_thresholds =
      !resolved.positive_hours_thresholds.empty() &&
      std::all_of(resolved.positive_hours_thresholds.begin(),
                  resolved.positive_hours_thresholds.end(),
                  [](double value) -> bool {
                    return std::isfinite(value) && value > 0;
                  }) &&
      std::adjacent_find(resolved.positive_hours_thresholds.begin(),
                         resolved.positive_hours_thresholds.end(),
                         [](double lhs, double rhs) -> bool {
                           return lhs >= rhs;
                         }) == resolved.positive_hours_thresholds.end();

  if (!has_valid_thresholds) {
    resolved.positive_hours_thresholds = {0.5, 1.0, 2.0, 4.0};
  }

  const std::size_t expected_palette_size =
      resolved.positive_hours_thresholds.size() + 1U;
  if (resolved.palette_colors.size() != expected_palette_size) {
    if (chart_theme == ReportChartTheme::kGithub) {
      resolved.palette_colors = {"#ebedf0", "#9be9a8", "#40c463", "#30a14e",
                                 "#216e39"};
    } else {
      resolved.palette_colors = {"#eef2ff", "#c7d2fe", "#818cf8", "#4f46e5",
                                 "#312e81"};
    }
  }

  return resolved;
}

[[nodiscard]] auto BuildReportChartHtml(
    const json &payload, ReportChartHtmlType chart_type,
    ReportChartTheme chart_theme,
    const ReportChartHeatmapVisualConfig &heatmap_visual_config)
    -> std::string {
  const std::string selected_root = payload.value("selected_root", "");
  const std::string root_label =
      selected_root.empty() ? std::string("all roots") : selected_root;
  const std::string range_label = BuildChartRangeLabel(payload);
  const std::string payload_json = EscapeInlineJsonScript(payload.dump());
  const auto &assets = LoadChartExporterAssets();
  const auto renderer = report_chart::CreateChartRenderer(chart_type);
  const auto renderer_spec = renderer->BuildSpec();
  const std::string chart_type_token(renderer_spec.kind_token);
  const std::string chart_title =
      "Report Chart (" + std::string(renderer_spec.title_label) + ")";
  const std::string page_title = "time_tracer report-chart " + chart_type_token;
  const std::string chart_theme_token =
      chart_theme == ReportChartTheme::kGithub ? "github" : "default";
  const std::string &chart_option_script =
      ResolveChartOptionScript(assets, chart_type);
  const ReportChartHeatmapVisualConfig resolved_heatmap_visual_config =
      BuildResolvedHeatmapVisualConfig(heatmap_visual_config, chart_theme);
  const std::string heatmap_palette_json = EscapeInlineJsonScript(
      json(resolved_heatmap_visual_config.palette_colors).dump());
  const std::string heatmap_positive_hours_json = EscapeInlineJsonScript(
      json(resolved_heatmap_visual_config.positive_hours_thresholds).dump());

  std::string html = assets.html_base_template;
  ReplaceTemplateToken(html, "{{PAGE_TITLE}}", EscapeHtml(page_title));
  ReplaceTemplateToken(html, "{{CHART_TITLE}}", EscapeHtml(chart_title));
  ReplaceTemplateToken(html, "{{CHART_KIND}}", chart_type_token);
  ReplaceTemplateToken(html, "{{CHART_THEME}}", chart_theme_token);
  ReplaceTemplateToken(html, "{{ROOT_LABEL}}", EscapeHtml(root_label));
  ReplaceTemplateToken(html, "{{RANGE_LABEL}}", EscapeHtml(range_label));
  ReplaceTemplateToken(html, "{{REPORT_CHART_JSON}}", payload_json);
  ReplaceTemplateToken(html, "{{HEATMAP_PALETTE_COLORS_JSON}}",
                       heatmap_palette_json);
  ReplaceTemplateToken(html, "{{HEATMAP_POSITIVE_HOURS_JSON}}",
                       heatmap_positive_hours_json);
  ReplaceTemplateToken(html, "{{CHART_OPTION_SCRIPT}}",
                       EscapeInlineScript(chart_option_script));
  ReplaceTemplateToken(html, "{{ECHARTS_SCRIPT}}",
                       EscapeInlineScript(assets.echarts_script));

  return html;
}

auto WriteChartHtmlFile(const fs::path &output_path, std::string_view html)
    -> void {
  const fs::path parent = output_path.parent_path();
  std::error_code io_error;
  if (!parent.empty()) {
    fs::create_directories(parent, io_error);
    if (io_error) {
      throw tracer_core::common::IoError(
          "Failed to create chart output directory: " + parent.string());
    }
  }

  std::ofstream file(output_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    throw tracer_core::common::IoError("Failed to open chart output file: " +
                                       output_path.string());
  }

  file.write(html.data(), static_cast<std::streamsize>(html.size()));
  file.flush();
  if (!file.good()) {
    throw tracer_core::common::IoError("Failed to write chart output file: " +
                                       output_path.string());
  }
}

} // namespace

auto ExportReportChartHtml(
    std::string_view semantic_payload, const fs::path &output_path,
    ReportChartHtmlType chart_type, ReportChartTheme chart_theme,
    const ReportChartHeatmapVisualConfig &heatmap_visual_config) -> void {
  const json payload = ParseReportChartPayloadForHtml(semantic_payload);
  const std::string html = BuildReportChartHtml(
      payload, chart_type, chart_theme, heatmap_visual_config);
  WriteChartHtmlFile(output_path, html);
}

auto ExportReportChartLineHtml(std::string_view semantic_payload,
                               const fs::path &output_path) -> void {
  ExportReportChartHtml(semantic_payload, output_path,
                        ReportChartHtmlType::kLine, ReportChartTheme::kDefault);
}

} // namespace tracer_core::cli::impl::presentation
