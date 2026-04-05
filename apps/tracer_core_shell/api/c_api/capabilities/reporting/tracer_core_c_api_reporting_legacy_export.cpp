import tracer.core.application.use_cases.interface;

#include "api/c_api/capabilities/reporting/tracer_core_c_api_reporting_internal.hpp"

#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "api/c_api/tracer_core_c_api.h"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/reporting_responses.hpp"
#include "shared/types/reporting_errors.hpp"

using tracer::core::application::use_cases::ITracerCoreRuntime;
using tracer_core::core::dto::ReportExportRequest;
using tracer_core::core::dto::ReportQueryRequest;
using tracer_core::core::dto::ReportQueryType;
using tracer_core::core::dto::ReportTargetType;
using tracer_core::core::dto::ReportTargetsOutput;
using tracer_core::core::dto::ReportTargetsRequest;
using tracer_core::core::dto::TextOutput;

namespace {

namespace fs = std::filesystem;

auto IsAllDigits(std::string_view value) -> bool {
  if (value.empty()) {
    return false;
  }
  for (const char character : value) {
    if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
      return false;
    }
  }
  return true;
}

auto ExtensionForFormat(ReportFormat format) -> std::string_view {
  switch (format) {
    case ReportFormat::kMarkdown:
      return ".md";
    case ReportFormat::kLaTeX:
      return ".tex";
    case ReportFormat::kTyp:
      return ".typ";
  }
  throw std::invalid_argument("Unsupported report format.");
}

auto DirectoryForFormat(ReportFormat format) -> std::string_view {
  switch (format) {
    case ReportFormat::kMarkdown:
      return "markdown";
    case ReportFormat::kLaTeX:
      return "latex";
    case ReportFormat::kTyp:
      return "typ";
  }
  throw std::invalid_argument("Unsupported report format.");
}

auto ShouldSkipLegacyCompatibilityWrite(std::string_view content) -> bool {
  return content.empty() ||
         content.find("No time records") != std::string_view::npos;
}

void WriteUtf8File(const fs::path& output_path, std::string_view content) {
  const fs::path parent = output_path.parent_path();
  if (!parent.empty()) {
    fs::create_directories(parent);
  }

  std::ofstream file(output_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file for writing: " +
                             output_path.string());
  }
  file.write(content.data(), static_cast<std::streamsize>(content.size()));
  if (file.fail()) {
    throw std::runtime_error("Failed to write file: " + output_path.string());
  }
}

auto BuildLegacyDayPath(const fs::path& export_root, ReportFormat format,
                        std::string_view date) -> fs::path {
  const fs::path base_dir = export_root / DirectoryForFormat(format) / "day";
  constexpr size_t kCompactDateLength = 8U;
  constexpr size_t kIsoDateLength = 10U;
  if (date.size() == kCompactDateLength && IsAllDigits(date)) {
    const std::string year(date.substr(0, 4));
    const std::string month(date.substr(4, 2));
    const std::string day(date.substr(6, 2));
    return base_dir / year / month / (year + "-" + month + "-" + day +
                                      std::string(ExtensionForFormat(format)));
  }
  if (date.size() == kIsoDateLength && date[4] == '-' && date[7] == '-' &&
      IsAllDigits(date.substr(0, 4)) && IsAllDigits(date.substr(5, 2)) &&
      IsAllDigits(date.substr(8, 2))) {
    return base_dir / std::string(date.substr(0, 4)) /
           std::string(date.substr(5, 2)) /
           (std::string(date) + std::string(ExtensionForFormat(format)));
  }
  return base_dir /
         (std::string(date) + std::string(ExtensionForFormat(format)));
}

auto BuildLegacyMonthPath(const fs::path& export_root, ReportFormat format,
                          std::string_view month) -> fs::path {
  return export_root / DirectoryForFormat(format) / "month" /
         (std::string(month) + std::string(ExtensionForFormat(format)));
}

auto BuildLegacyRecentPath(const fs::path& export_root, ReportFormat format,
                           int days) -> fs::path {
  return export_root / DirectoryForFormat(format) / "recent" /
         ("last_" + std::to_string(days) + "_days_report" +
          std::string(ExtensionForFormat(format)));
}

auto BuildLegacyWeekPath(const fs::path& export_root, ReportFormat format,
                         std::string_view iso_week) -> fs::path {
  return export_root / DirectoryForFormat(format) / "week" /
         (std::string(iso_week) + std::string(ExtensionForFormat(format)));
}

auto BuildLegacyYearPath(const fs::path& export_root, ReportFormat format,
                         std::string_view year) -> fs::path {
  return export_root / DirectoryForFormat(format) / "year" /
         (std::string(year) + std::string(ExtensionForFormat(format)));
}

auto CanonicalMonthToLegacyAllMonthId(std::string_view month) -> std::string {
  if (month.size() == 7U && month[4] == '-' && IsAllDigits(month.substr(0, 4)) &&
      IsAllDigits(month.substr(5, 2))) {
    return std::string(month.substr(0, 4)) + std::string(month.substr(5, 2));
  }
  return std::string(month);
}

auto RequireArgument(const ReportExportRequest& request) -> const std::string& {
  if (request.argument.empty()) {
    throw std::invalid_argument(
        "report export request requires non-empty `argument`.");
  }
  return request.argument;
}

auto RequireDaysList(const ReportExportRequest& request)
    -> const std::vector<int>& {
  if (request.recent_days_list.empty()) {
    throw std::invalid_argument(
        "report export request requires non-empty `recent_days_list`.");
  }
  return request.recent_days_list;
}

auto RequireOkText(const TextOutput& output, std::string_view context)
    -> std::string {
  if (!output.ok) {
    if (!output.error_contract.error_code.empty()) {
      throw tracer_core::common::ReportingContractError(
          output.error_message.empty() ? std::string(context) + " failed."
                                       : output.error_message,
          output.error_contract.error_code,
          output.error_contract.error_category,
          output.error_contract.hints);
    }
    if (!output.error_message.empty()) {
      throw std::runtime_error(output.error_message);
    }
    throw std::runtime_error(std::string(context) + " failed.");
  }
  return output.content;
}

auto RequireOkTargets(const ReportTargetsOutput& output,
                      std::string_view context) -> std::vector<std::string> {
  if (!output.ok) {
    if (!output.error_contract.error_code.empty()) {
      throw tracer_core::common::ReportingContractError(
          output.error_message.empty() ? std::string(context) + " failed."
                                       : output.error_message,
          output.error_contract.error_code,
          output.error_contract.error_category,
          output.error_contract.hints);
    }
    if (!output.error_message.empty()) {
      throw std::runtime_error(output.error_message);
    }
    throw std::runtime_error(std::string(context) + " failed.");
  }
  return output.items;
}

auto RenderReport(ITracerCoreRuntime& runtime, ReportQueryType type,
                  std::string argument, ReportFormat format) -> std::string {
  return RequireOkText(runtime.report().RunReportQuery(ReportQueryRequest{
                           .type = type,
                           .argument = std::move(argument),
                           .format = format,
                       }),
                       "runtime report render");
}

auto ListTargets(ITracerCoreRuntime& runtime, ReportTargetType type)
    -> std::vector<std::string> {
  return RequireOkTargets(
      runtime.report().RunReportTargetsQuery(ReportTargetsRequest{.type = type}),
      "runtime report target listing");
}

void WriteLegacyCompatibilityFile(const fs::path& output_path,
                                  std::string_view content) {
  if (ShouldSkipLegacyCompatibilityWrite(content)) {
    return;
  }
  WriteUtf8File(output_path, content);
}

}  // namespace

namespace tracer_core::core::c_api::reporting {

void ExportSpecificLegacyCompatibility(ITracerCoreRuntime& runtime,
                                       const fs::path& export_root,
                                       const ReportExportRequest& request) {
  switch (request.type) {
    case tracer_core::core::dto::ReportExportType::kDay:
      WriteLegacyCompatibilityFile(
          BuildLegacyDayPath(export_root, request.format, RequireArgument(request)),
          RenderReport(runtime, ReportQueryType::kDay, request.argument,
                       request.format));
      return;
    case tracer_core::core::dto::ReportExportType::kMonth:
      WriteLegacyCompatibilityFile(BuildLegacyMonthPath(export_root, request.format,
                                                        RequireArgument(request)),
                                   RenderReport(runtime, ReportQueryType::kMonth,
                                                request.argument, request.format));
      return;
    case tracer_core::core::dto::ReportExportType::kRecent: {
      const std::string& argument = RequireArgument(request);
      const int days = std::stoi(argument);
      WriteLegacyCompatibilityFile(
          BuildLegacyRecentPath(export_root, request.format, days),
          RenderReport(runtime, ReportQueryType::kRecent, argument, request.format));
      return;
    }
    case tracer_core::core::dto::ReportExportType::kWeek:
      WriteLegacyCompatibilityFile(BuildLegacyWeekPath(export_root, request.format,
                                                       RequireArgument(request)),
                                   RenderReport(runtime, ReportQueryType::kWeek,
                                                request.argument, request.format));
      return;
    case tracer_core::core::dto::ReportExportType::kYear:
      WriteLegacyCompatibilityFile(BuildLegacyYearPath(export_root, request.format,
                                                       RequireArgument(request)),
                                   RenderReport(runtime, ReportQueryType::kYear,
                                                request.argument, request.format));
      return;
    case tracer_core::core::dto::ReportExportType::kAllDay:
    case tracer_core::core::dto::ReportExportType::kAllMonth:
    case tracer_core::core::dto::ReportExportType::kAllRecent:
    case tracer_core::core::dto::ReportExportType::kAllWeek:
    case tracer_core::core::dto::ReportExportType::kAllYear:
      break;
  }
  throw std::invalid_argument("Export request is not a specific report export.");
}

void ExportAllLegacyCompatibility(ITracerCoreRuntime& runtime,
                                  const fs::path& export_root,
                                  const ReportExportRequest& request) {
  using tracer_core::core::dto::ReportExportType;

  switch (request.type) {
    case ReportExportType::kAllDay:
      for (const std::string& target : ListTargets(runtime, ReportTargetType::kDay)) {
        WriteLegacyCompatibilityFile(
            BuildLegacyDayPath(export_root, request.format, target),
            RenderReport(runtime, ReportQueryType::kDay, target, request.format));
      }
      return;
    case ReportExportType::kAllMonth:
      for (const std::string& target :
           ListTargets(runtime, ReportTargetType::kMonth)) {
        WriteLegacyCompatibilityFile(
            BuildLegacyMonthPath(export_root, request.format,
                                 CanonicalMonthToLegacyAllMonthId(target)),
            RenderReport(runtime, ReportQueryType::kMonth, target, request.format));
      }
      return;
    case ReportExportType::kAllRecent:
      for (const int days : RequireDaysList(request)) {
        WriteLegacyCompatibilityFile(
            BuildLegacyRecentPath(export_root, request.format, days),
            RenderReport(runtime, ReportQueryType::kRecent, std::to_string(days),
                         request.format));
      }
      return;
    case ReportExportType::kAllWeek:
      for (const std::string& target : ListTargets(runtime, ReportTargetType::kWeek)) {
        WriteLegacyCompatibilityFile(
            BuildLegacyWeekPath(export_root, request.format, target),
            RenderReport(runtime, ReportQueryType::kWeek, target, request.format));
      }
      return;
    case ReportExportType::kAllYear:
      for (const std::string& target : ListTargets(runtime, ReportTargetType::kYear)) {
        WriteLegacyCompatibilityFile(
            BuildLegacyYearPath(export_root, request.format, target),
            RenderReport(runtime, ReportQueryType::kYear, target, request.format));
      }
      return;
    case ReportExportType::kDay:
    case ReportExportType::kMonth:
    case ReportExportType::kRecent:
    case ReportExportType::kWeek:
    case ReportExportType::kYear:
      break;
  }
  throw std::invalid_argument("Export request is not an all-reports export.");
}

}  // namespace tracer_core::core::c_api::reporting
