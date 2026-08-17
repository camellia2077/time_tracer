#include "application/use_cases/insights_api_support.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "domain/utils/time_utils.hpp"
#include "shared/types/insights_errors.hpp"
#include "shared/utils/period_utils.hpp"

namespace tracer::core::application::use_cases::insights_support {

namespace fs = std::filesystem;
using ::InsightsFormat;

auto ExtensionForFormat(InsightsFormat format) -> std::string_view {
  switch (format) {
    case InsightsFormat::kMarkdown:
      return ".md";
    case InsightsFormat::kLaTeX:
      return ".tex";
    case InsightsFormat::kTyp:
      return ".typ";
  }
  throw std::invalid_argument("Unsupported insights format.");
}

auto DirectoryForFormat(InsightsFormat format) -> std::string_view {
  switch (format) {
    case InsightsFormat::kMarkdown:
      return "markdown";
    case InsightsFormat::kLaTeX:
      return "latex";
    case InsightsFormat::kTyp:
      return "typ";
  }
  throw std::invalid_argument("Unsupported insights format.");
}

auto BuildDayPath(const fs::path& export_root, InsightsFormat format,
                  std::string_view date) -> fs::path {
  const fs::path kBaseDir = export_root / DirectoryForFormat(format) / "day";
  if (date.size() == 10U) {
    return kBaseDir / std::string(date.substr(0, 4)) /
           std::string(date.substr(5, 2)) /
           (std::string(date) + std::string(ExtensionForFormat(format)));
  }
  return kBaseDir /
         (std::string(date) + std::string(ExtensionForFormat(format)));
}

auto BuildMonthPath(const fs::path& export_root, InsightsFormat format,
                    std::string_view month) -> fs::path {
  return export_root / DirectoryForFormat(format) / "month" /
         (std::string(month) + std::string(ExtensionForFormat(format)));
}

auto BuildRecentPath(const fs::path& export_root, InsightsFormat format,
                     int days) -> fs::path {
  return export_root / DirectoryForFormat(format) / "recent" /
         ("last_" + std::to_string(days) + "_days_insights" +
          std::string(ExtensionForFormat(format)));
}

auto BuildWeekPath(const fs::path& export_root, InsightsFormat format,
                   std::string_view iso_week) -> fs::path {
  return export_root / DirectoryForFormat(format) / "week" /
         (std::string(iso_week) + std::string(ExtensionForFormat(format)));
}

auto BuildYearPath(const fs::path& export_root, InsightsFormat format,
                   std::string_view year) -> fs::path {
  return export_root / DirectoryForFormat(format) / "year" /
         (std::string(year) + std::string(ExtensionForFormat(format)));
}

auto BuildRangePath(const fs::path& export_root, InsightsFormat format,
                    std::string_view start_date, std::string_view end_date)
    -> fs::path {
  return export_root / DirectoryForFormat(format) / "range" /
         (std::string(start_date) + "_" + std::string(end_date) +
          std::string(ExtensionForFormat(format)));
}

auto ShouldSkipExportWrite(std::string_view content) -> bool {
  return content.empty() ||
         content.find("No time records") != std::string_view::npos;
}

auto WriteUtf8File(const fs::path& output_path, std::string_view content)
    -> void {
  const fs::path kParent = output_path.parent_path();
  if (!kParent.empty()) {
    fs::create_directories(kParent);
  }

  std::ofstream file(output_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    throw tracer_core::common::InsightsContractError(
        "Unable to open insights export file: " + output_path.string(),
        "export.write.failed", "export",
        {"Check that the export output directory is writable."});
  }
  file.write(content.data(), static_cast<std::streamsize>(content.size()));
  if (file.fail()) {
    throw tracer_core::common::InsightsContractError(
        "Failed to write insights export file: " + output_path.string(),
        "export.write.failed", "export",
        {"Check that the export output directory is writable."});
  }
}

auto WriteExportFileIfNeeded(const fs::path& output_path,
                             std::string_view content) -> void {
  if (ShouldSkipExportWrite(content)) {
    return;
  }
  WriteUtf8File(output_path, content);
}

}  // namespace tracer::core::application::use_cases::insights_support
