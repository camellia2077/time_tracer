// reports/range/formatters/markdown/range_md_formatter.cpp
#include "range_md_formatter.hpp"

#include <format>

#include <toml++/toml.h>

#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

RangeMdFormatter::RangeMdFormatter(std::shared_ptr<RangeMdConfig> config)
    : BaseMdFormatter(config) {}

std::string RangeMdFormatter::validate_data(const RangeReportData& data) const {
  if (!data.is_valid) {
    return config_->get_invalid_range_message();
  }
  return "";
}

bool RangeMdFormatter::is_empty_data(const RangeReportData& data) const {
  return data.actual_days == 0;
}

int RangeMdFormatter::get_avg_days(const RangeReportData& data) const {
  return data.actual_days;
}

std::string RangeMdFormatter::get_no_records_msg() const {
  return config_->get_no_records_message();
}

void RangeMdFormatter::format_header_content(
    std::stringstream& ss, const RangeReportData& data) const {
  std::string title =
      format_title_template(config_->get_title_template(), data);
  ss << std::format("## {}\n\n", title);

  if (data.actual_days > 0) {
    ss << std::format("- **{0}**: {1}\n", config_->get_total_time_label(),
                      time_format_duration(data.total_duration,
                                            data.actual_days));
    ss << std::format("- **{0}**: {1}\n", config_->get_actual_days_label(),
                      data.actual_days);
  }
}

extern "C" {
  __declspec(dllexport) FormatterHandle create_formatter(
      const char* config_toml) {
    try {
      auto config_tbl = toml::parse(config_toml);
      auto md_config = std::make_shared<RangeMdConfig>(config_tbl);
      auto formatter = new RangeMdFormatter(md_config);
      return static_cast<FormatterHandle>(formatter);
    } catch (...) {
      return nullptr;
    }
  }

  __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
    if (handle) {
      delete static_cast<RangeMdFormatter*>(handle);
    }
  }

  static std::string report_buffer;

  __declspec(dllexport) const char* format_report(FormatterHandle handle,
                                                  const RangeReportData& data) {
    if (handle) {
      auto* formatter = static_cast<RangeMdFormatter*>(handle);
      report_buffer = formatter->format_report(data);
      return report_buffer.c_str();
    }
    return "";
  }
}
