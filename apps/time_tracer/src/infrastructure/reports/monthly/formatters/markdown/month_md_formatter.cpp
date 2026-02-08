// infrastructure/reports/monthly/formatters/markdown/month_md_formatter.cpp
#include "infrastructure/reports/monthly/formatters/markdown/month_md_formatter.hpp"

#include <toml++/toml.h>

#include <format>
#include <memory>

#include "infrastructure/reports/shared/utils/format/report_string_utils.hpp"
#include "infrastructure/reports/shared/utils/format/time_format.hpp"

namespace {
auto FormatRatio(int count, int total_days) -> std::string {
  if (total_days <= 0) {
    return std::to_string(count);
  }
  double percent =
      100.0 * static_cast<double>(count) / static_cast<double>(total_days);
  return std::format("{} ({:.2f}%)", count, percent);
}
}  // namespace

MonthMdFormatter::MonthMdFormatter(std::shared_ptr<MonthMdConfig> config)
    : BaseMdFormatter(config) {}

auto MonthMdFormatter::ValidateData(const MonthlyReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage();
  }
  return "";
}

auto MonthMdFormatter::IsEmptyData(const MonthlyReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthMdFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
  return data.actual_days;
}

auto MonthMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthMdFormatter::FormatHeaderContent(
    std::stringstream& report_stream, const MonthlyReportData& data) const {
  std::string title = FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream << std::format("## {}\n\n", title);

  if (data.actual_days > 0) {
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetActualDaysLabel(), data.actual_days);
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetStatusDaysLabel(),
        FormatRatio(data.status_true_days, data.actual_days));
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetSleepDaysLabel(),
        FormatRatio(data.sleep_true_days, data.actual_days));
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetExerciseDaysLabel(),
        FormatRatio(data.exercise_true_days, data.actual_days));
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetCardioDaysLabel(),
        FormatRatio(data.cardio_true_days, data.actual_days));
    report_stream << std::format(
        "- **{0}**: {1}\n", config_->GetAnaerobicDaysLabel(),
        FormatRatio(data.anaerobic_true_days, data.actual_days));
  }
}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
__declspec(dllexport) FormatterHandle
CreateFormatter(const char* config_toml) {  // NOLINT
  try {
    // [FIX] 使用 toml::parse
    auto config_tbl = toml::parse(config_toml);
    auto md_config = std::make_shared<MonthMdConfig>(config_tbl);
    auto formatter = std::make_unique<MonthMdFormatter>(md_config);
    return static_cast<FormatterHandle>(formatter.release());
  } catch (...) {
    return nullptr;
  }
}
// NOLINTEND(readability-identifier-naming)

__declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {  // NOLINT
  if (handle != nullptr) {
    std::unique_ptr<MonthMdFormatter>{static_cast<MonthMdFormatter*>(handle)};
  }
}

static std::string report_buffer;

__declspec(dllexport) auto FormatReport(
    FormatterHandle handle,
    const MonthlyReportData& data)  // NOLINT
    -> const char* {
  if (handle != nullptr) {
    auto* formatter = static_cast<MonthMdFormatter*>(handle);
    report_buffer = formatter->FormatReport(data);
    return report_buffer.c_str();
  }
  return kEmptyReport;
}
}
