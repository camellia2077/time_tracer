// reports/range/formatters/markdown/range_md_formatter.cpp
#include "range_md_formatter.hpp"

#include <format>

#include <toml++/toml.h>

#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

RangeMdFormatter::RangeMdFormatter(std::shared_ptr<RangeMdConfig> config)
    : BaseMdFormatter(config) {}

auto RangeMdFormatter::validate_data(const RangeReportData& data) const -> std::string {
  if (!data.is_valid) {
    return config_->get_invalid_range_message();
  }
  return std::string{};
}

auto RangeMdFormatter::is_empty_data(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeMdFormatter::get_avg_days(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeMdFormatter::get_no_records_msg() const -> std::string {
  return config_->get_no_records_message();
}

void RangeMdFormatter::format_header_content(
    std::stringstream& report_stream, const RangeReportData& data) const {
  std::string title =
      format_title_template(config_->get_title_template(), data);
  report_stream << std::format("## {}\n\n", title);

  if (data.actual_days > 0) {
    report_stream << std::format("- **{0}**: {1}\n", config_->get_total_time_label(),
                      time_format_duration(data.total_duration,
                                            data.actual_days));
    report_stream << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(),
                      data.actual_days);
  }
}

extern "C" {
  // Required exports: create_formatter / destroy_formatter / format_report (lowercase)
  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto create_formatter(
      const char* config_toml) -> FormatterHandle {
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
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
    if (handle != nullptr) {
      std::unique_ptr<RangeMdFormatter>{
          static_cast<RangeMdFormatter*>(handle)};
    }
  }

  static std::string report_buffer;

  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto format_report(FormatterHandle handle,
                                           const RangeReportData& data) -> const char* {
    if (handle != nullptr) {
      auto* formatter = static_cast<RangeMdFormatter*>(handle);
      report_buffer = formatter->format_report(data);
      return report_buffer.c_str();
    }
    return kEmptyReport;
  }

  // Backward-compatible exports (legacy symbol names)
  __declspec(dllexport) auto CreateFormatter(
      const char* config_toml) -> FormatterHandle {
    return create_formatter(config_toml);
  }

  __declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {
    destroy_formatter(handle);
  }

  __declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                          const RangeReportData& data) -> const char* {
    return format_report(handle, data);
  }
}
