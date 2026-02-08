// infrastructure/reports/range/formatters/typst/range_typ_formatter.cpp
#include "infrastructure/reports/range/formatters/typst/range_typ_formatter.hpp"

#include <toml++/toml.h>

#include <format>

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

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

RangeTypFormatter::RangeTypFormatter(std::shared_ptr<RangeTypConfig> config)
    : BaseTypFormatter(config) {}

auto RangeTypFormatter::ValidateData(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeTypFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeTypFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTypFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTypFormatter::FormatPageSetup(
    std::stringstream& report_stream) const {
  report_stream
      << std::format(
             R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
             config_->GetMarginTopCm(), config_->GetMarginBottomCm(),
             config_->GetMarginLeftCm(), config_->GetMarginRightCm())
      << "\n";
}

void RangeTypFormatter::FormatHeaderContent(std::stringstream& report_stream,
                                            const RangeReportData& data) const {
  std::string title = FormatTitleTemplate(config_->GetTitleTemplate(), data);
  std::string title_line = std::format(
      R"(#text(font: "{}", size: {}pt)[= {}])", config_->GetTitleFont(),
      config_->GetReportTitleFontSize(), title);
  report_stream << title_line << "\n\n";

  if (data.actual_days > 0) {
    report_stream << std::format(
        "+ *{}:* {}\n", config_->GetTotalTimeLabel(),
        TimeFormatDuration(data.total_duration, data.actual_days));
    report_stream << std::format("+ *{}:* {}\n", config_->GetActualDaysLabel(),
                                 data.actual_days);
    report_stream << std::format("+ *{}:* {}\n", config_->GetStatusDaysLabel(),
                                 FormatRatio(data.status_true_days,
                                             data.actual_days));
    report_stream << std::format("+ *{}:* {}\n", config_->GetSleepDaysLabel(),
                                 FormatRatio(data.sleep_true_days,
                                             data.actual_days));
    report_stream << std::format("+ *{}:* {}\n",
                                 config_->GetExerciseDaysLabel(),
                                 FormatRatio(data.exercise_true_days,
                                             data.actual_days));
    report_stream << std::format("+ *{}:* {}\n", config_->GetCardioDaysLabel(),
                                 FormatRatio(data.cardio_true_days,
                                             data.actual_days));
    report_stream << std::format("+ *{}:* {}\n",
                                 config_->GetAnaerobicDaysLabel(),
                                 FormatRatio(data.anaerobic_true_days,
                                             data.actual_days));
  }
}

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
// Public API: keep symbol name stable for dynamic loading.
__declspec(dllexport) auto CreateFormatter(const char* config_toml)
    -> FormatterHandle {  // NOLINT
  try {
    auto config_tbl = toml::parse(config_toml);
    auto typ_config = std::make_shared<RangeTypConfig>(config_tbl);
    auto formatter = std::make_unique<RangeTypFormatter>(typ_config);
    return static_cast<FormatterHandle>(formatter.release());
  } catch (...) {
    return nullptr;
  }
}
// NOLINTEND(readability-identifier-naming)

// Public API: keep symbol name stable for dynamic loading.
__declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {  // NOLINT
  if (handle != nullptr) {
    std::unique_ptr<RangeTypFormatter>{static_cast<RangeTypFormatter*>(handle)};
  }
}

static std::string report_buffer;

__declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                        const RangeReportData& data) -> const
    char* {  // NOLINT
  if (handle != nullptr) {
    auto* formatter = static_cast<RangeTypFormatter*>(handle);
    report_buffer = formatter->FormatReport(data);
    return report_buffer.c_str();
  }
  return kEmptyReport;
}
}
