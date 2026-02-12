// infrastructure/reports/plugin_manifest.hpp
#ifndef INFRASTRUCTURE_REPORTS_PLUGIN_MANIFEST_HPP_
#define INFRASTRUCTURE_REPORTS_PLUGIN_MANIFEST_HPP_

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "domain/reports/types/report_format.hpp"

namespace reports::plugin_manifest {

enum class ReportKind { kDay, kMonth, kPeriod, kWeek, kYear };

struct FormatterPluginBinding {
  ReportKind report_kind;
  ReportFormat format;
  std::string_view plugin_name;
};

constexpr std::string_view kCoreRuntimeLibraryName = "reports_shared";

[[nodiscard]] auto GetFormatterPluginBindings()
    -> std::span<const FormatterPluginBinding>;

[[nodiscard]] auto GetExpectedFormatterPluginNames()
    -> std::vector<std::string>;

}  // namespace reports::plugin_manifest

#endif  // INFRASTRUCTURE_REPORTS_PLUGIN_MANIFEST_HPP_
