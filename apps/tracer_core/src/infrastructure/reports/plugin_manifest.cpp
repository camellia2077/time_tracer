// infrastructure/reports/plugin_manifest.cpp
#include "infrastructure/reports/plugin_manifest.hpp"

#include <algorithm>
#include <array>

namespace reports::plugin_manifest {
namespace {
constexpr std::array<FormatterPluginBinding, 15> kFormatterPluginBindings = {{
    {.report_kind = ReportKind::kDay,
     .format = ReportFormat::kMarkdown,
     .plugin_name = "DayMdFormatter"},
    {.report_kind = ReportKind::kDay,
     .format = ReportFormat::kTyp,
     .plugin_name = "DayTypFormatter"},
    {.report_kind = ReportKind::kDay,
     .format = ReportFormat::kLaTeX,
     .plugin_name = "DayTexFormatter"},

    {.report_kind = ReportKind::kMonth,
     .format = ReportFormat::kMarkdown,
     .plugin_name = "MonthMdFormatter"},
    {.report_kind = ReportKind::kMonth,
     .format = ReportFormat::kTyp,
     .plugin_name = "MonthTypFormatter"},
    {.report_kind = ReportKind::kMonth,
     .format = ReportFormat::kLaTeX,
     .plugin_name = "MonthTexFormatter"},

    {.report_kind = ReportKind::kPeriod,
     .format = ReportFormat::kMarkdown,
     .plugin_name = "RangeMdFormatter"},
    {.report_kind = ReportKind::kPeriod,
     .format = ReportFormat::kLaTeX,
     .plugin_name = "RangeTexFormatter"},
    {.report_kind = ReportKind::kPeriod,
     .format = ReportFormat::kTyp,
     .plugin_name = "RangeTypFormatter"},

    {.report_kind = ReportKind::kWeek,
     .format = ReportFormat::kMarkdown,
     .plugin_name = "RangeMdFormatter"},
    {.report_kind = ReportKind::kWeek,
     .format = ReportFormat::kLaTeX,
     .plugin_name = "RangeTexFormatter"},
    {.report_kind = ReportKind::kWeek,
     .format = ReportFormat::kTyp,
     .plugin_name = "RangeTypFormatter"},

    {.report_kind = ReportKind::kYear,
     .format = ReportFormat::kMarkdown,
     .plugin_name = "RangeMdFormatter"},
    {.report_kind = ReportKind::kYear,
     .format = ReportFormat::kLaTeX,
     .plugin_name = "RangeTexFormatter"},
    {.report_kind = ReportKind::kYear,
     .format = ReportFormat::kTyp,
     .plugin_name = "RangeTypFormatter"},
}};
}  // namespace

auto GetFormatterPluginBindings() -> std::span<const FormatterPluginBinding> {
  return kFormatterPluginBindings;
}

auto GetExpectedFormatterPluginNames() -> std::vector<std::string> {
  std::vector<std::string> plugin_names;
  plugin_names.reserve(kFormatterPluginBindings.size());

  for (const auto& binding : kFormatterPluginBindings) {
    const std::string kPluginName(binding.plugin_name);
    if (std::ranges::find(plugin_names, kPluginName) == plugin_names.end()) {
      plugin_names.push_back(kPluginName);
    }
  }

  return plugin_names;
}

}  // namespace reports::plugin_manifest
