// reports/range/formatters/latex/range_tex_formatter.cpp
#include "range_tex_formatter.hpp"

#include <toml++/toml.h>

#include "reports/range/formatters/latex/range_tex_utils.hpp"

RangeTexFormatter::RangeTexFormatter(std::shared_ptr<RangeTexConfig> config)
    : BaseTexFormatter(config) {}

std::string RangeTexFormatter::validate_data(const RangeReportData& data) const {
  if (!data.is_valid) {
    return config_->get_invalid_range_message() + "\n";
  }
  return "";
}

bool RangeTexFormatter::is_empty_data(const RangeReportData& data) const {
  return data.actual_days == 0;
}

int RangeTexFormatter::get_avg_days(const RangeReportData& data) const {
  return data.actual_days;
}

std::string RangeTexFormatter::get_no_records_msg() const {
  return config_->get_no_records_message();
}

void RangeTexFormatter::format_header_content(
    std::stringstream& ss, const RangeReportData& data) const {
  RangeTexUtils::display_summary(ss, data, config_);
}

extern "C" {
  __declspec(dllexport) FormatterHandle create_formatter(
      const char* config_toml) {
    try {
      auto config_tbl = toml::parse(config_toml);
      auto tex_config = std::make_shared<RangeTexConfig>(config_tbl);
      auto formatter = new RangeTexFormatter(tex_config);
      return static_cast<FormatterHandle>(formatter);
    } catch (...) {
      return nullptr;
    }
  }

  __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
    if (handle) {
      delete static_cast<RangeTexFormatter*>(handle);
    }
  }

  static std::string report_buffer;

  __declspec(dllexport) const char* format_report(FormatterHandle handle,
                                                  const RangeReportData& data) {
    if (handle) {
      auto* formatter = static_cast<RangeTexFormatter*>(handle);
      report_buffer = formatter->format_report(data);
      return report_buffer.c_str();
    }
    return "";
  }
}
