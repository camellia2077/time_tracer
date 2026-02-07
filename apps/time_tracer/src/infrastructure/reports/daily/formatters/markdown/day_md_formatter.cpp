// infrastructure/reports/daily/formatters/markdown/day_md_formatter.cpp
#include "infrastructure/reports/daily/formatters/markdown/day_md_formatter.hpp"

#include <toml++/toml.h>

#include <format>
#include <iomanip>
#include <memory>
#include <string>

#include "infrastructure/reports/daily/formatters/statistics/markdown_stat_strategy.hpp"
#include "infrastructure/reports/daily/formatters/statistics/stat_formatter.hpp"
#include "infrastructure/reports/shared/utils/format/bool_to_string.hpp"
#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {
constexpr int kRemarkIndent = 2;
const std::string kRemarkIndentPrefix = "  ";
constexpr const char* kEmptyReport = "";
}  // namespace

DayMdFormatter::DayMdFormatter(std::shared_ptr<DayMdConfig> config)
    : BaseMdFormatter(config) {}

auto DayMdFormatter::IsEmptyData(const DailyReportData& data) const -> bool {
  return data.total_duration == 0;
}

auto DayMdFormatter::GetAvgDays(const DailyReportData& /*data*/) const -> int {
  return 1;
}

auto DayMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecords();
}

void DayMdFormatter::FormatHeaderContent(std::stringstream& report_stream,
                                         const DailyReportData& data) const {
  report_stream << std::format("## {0} {1}\n\n", config_->GetTitlePrefix(),
                               data.date);
  report_stream << std::format("- **{0}**: {1}\n", config_->GetDateLabel(),
                               data.date);
  report_stream << std::format("- **{0}**: {1}\n", config_->GetTotalTimeLabel(),
                               TimeFormatDuration(data.total_duration));
  report_stream << std::format("- **{0}**: {1}\n", config_->GetStatusLabel(),
                               BoolToString(data.metadata.status));
  report_stream << std::format("- **{0}**: {1}\n", config_->GetSleepLabel(),
                               BoolToString(data.metadata.sleep));
  report_stream << std::format("- **{0}**: {1}\n", config_->GetExerciseLabel(),
                               BoolToString(data.metadata.exercise));
  report_stream << std::format("- **{0}**: {1}\n", config_->GetGetupTimeLabel(),
                               data.metadata.getup_time);

  std::string formatted_remark = FormatMultilineForList(
      data.metadata.remark, kRemarkIndent, kRemarkIndentPrefix);
  report_stream << std::format("- **{0}**: {1}\n", config_->GetRemarkLabel(),
                               formatted_remark);
}

void DayMdFormatter::FormatExtraContent(std::stringstream& report_stream,
                                        const DailyReportData& data) const {
  auto strategy = std::make_unique<MarkdownStatStrategy>();
  StatFormatter stats_formatter(std::move(strategy));
  report_stream << stats_formatter.Format(data, config_);
  DisplayDetailedActivities(report_stream, data);
}

void DayMdFormatter::DisplayDetailedActivities(
    std::stringstream& report_stream, const DailyReportData& data) const {
  if (!data.detailed_records.empty()) {
    report_stream << "\n## " << config_->GetAllActivitiesLabel() << "\n\n";
    for (const auto& record : data.detailed_records) {
      std::string project_path =
          ReplaceAll(record.project_path, "_", config_->GetActivityConnector());
      report_stream << std::format(
          "- {0} - {1} ({2}): {3}\n", record.start_time, record.end_time,
          TimeFormatDuration(record.duration_seconds), project_path);
      if (record.activityRemark.has_value()) {
        report_stream << std::format("  - **{0}**: {1}\n",
                                     config_->GetActivityRemarkLabel(),
                                     record.activityRemark.value());
      }
    }
    report_stream << "\n";
  }
}

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
// [核心修改] 解析 TOML 字符串
// Public API: keep symbol name stable for dynamic loading.
__declspec(dllexport) auto CreateFormatter(const char* config_content)
    -> FormatterHandle {  // NOLINT
  try {
    auto config_tbl = toml::parse(config_content);
    auto md_config = std::make_shared<DayMdConfig>(config_tbl);
    auto formatter = std::make_unique<DayMdFormatter>(md_config);
    return static_cast<FormatterHandle>(formatter.release());
  } catch (...) {
    return nullptr;
  }
}
// NOLINTEND(readability-identifier-naming)

__declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {  // NOLINT
  if (handle != nullptr) {
    std::unique_ptr<DayMdFormatter>{static_cast<DayMdFormatter*>(handle)};
  }
}

static std::string report_buffer;

__declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                        const DailyReportData& data) -> const
    char* {  // NOLINT
  if (handle != nullptr) {
    auto* formatter = static_cast<DayMdFormatter*>(handle);
    report_buffer = formatter->FormatReport(data);
    return report_buffer.c_str();
  }
  return kEmptyReport;
}
}
