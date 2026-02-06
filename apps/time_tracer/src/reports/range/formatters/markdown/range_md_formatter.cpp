// reports/range/formatters/markdown/range_md_formatter.cpp
#include "reports/range/formatters/markdown/range_md_formatter.hpp"

#include <format>

#include <toml++/toml.h>

#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

RangeMdFormatter::RangeMdFormatter(std::shared_ptr<RangeMdConfig> config)
    : BaseMdFormatter(config) {}

auto RangeMdFormatter::ValidateData(const RangeReportData& data) const -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage();
  }
  return std::string{};
}

auto RangeMdFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeMdFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeMdFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeMdFormatter::FormatHeaderContent(
    std::stringstream& report_stream, const RangeReportData& data) const {
  std::string title =
      FormatTitleTemplate(config_->GetTitleTemplate(), data);
  report_stream << std::format("## {}\n\n", title);

  if (data.actual_days > 0) {
    report_stream << std::format("- **{0}**: {1}\n", config_->GetTotalTimeLabel(),
                      TimeFormatDuration(data.total_duration,
                                            data.actual_days));
    report_stream << std::format("- **{0}**: {1}\n", config_->GetActualDaysLabel(),
                      data.actual_days);
  }
}

extern "C" {
  // Required exports: create_formatter / destroy_formatter / format_report (lowercase)
  // Public API: keep symbol name stable for dynamic loading.
  __declspec(dllexport) auto CreateFormatter(
      const char* config_toml) -> FormatterHandle { // NOLINT
    try {
      auto config_tbl = toml::parse(config_toml);
      auto md_config = std::make_shared<RangeMdConfig>(config_tbl);
      auto formatter = std::make_unique<RangeMdFormatter>(md_config);
      return static_cast<FormatterHandle>(formatter.release());
    } catch (...) {
      return nullptr;
    }
  }

  // Public API: keep symbol name stable for dynamic loading.
  __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) { // NOLINT
    if (handle != nullptr) {
      std::unique_ptr<RangeMdFormatter>{
          static_cast<RangeMdFormatter*>(handle)};
    }
  }

  static std::string report_buffer;

  // Public API: keep symbol name stable for dynamic loading.
  __declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                           const RangeReportData& data) -> const char* { // NOLINT
    if (handle != nullptr) {
      auto* formatter = static_cast<RangeMdFormatter*>(handle);
      report_buffer = formatter->FormatReport(data);
      return report_buffer.c_str();
    }
    return kEmptyReport;
  }

  // Backward-compatible exports (legacy symbol names)
  __declspec(dllexport) auto createFormatter_Legacy(
      const char* config_toml) -> FormatterHandle { // NOLINT
    return CreateFormatter(config_toml);
  }

  __declspec(dllexport) void destroyFormatter_Legacy(FormatterHandle handle) { // NOLINT
    DestroyFormatter(handle);
  }

  __declspec(dllexport) auto formatReport_Legacy(FormatterHandle handle,
                                          const RangeReportData& data) -> const char* { // NOLINT
    return FormatReport(handle, data);
  }
}
