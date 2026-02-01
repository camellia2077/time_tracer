// reports/range/formatters/typst/range_typ_formatter.cpp
#include "range_typ_formatter.hpp"

#include <format>

#include <toml++/toml.h>

#include "reports/shared/utils/format/report_string_utils.hpp"
#include "reports/shared/utils/format/time_format.hpp"

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

RangeTypFormatter::RangeTypFormatter(std::shared_ptr<RangeTypConfig> config)
    : BaseTypFormatter(config) {}

auto RangeTypFormatter::validate_data(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->get_invalid_range_message();
  }
  return std::string{};
}

auto RangeTypFormatter::is_empty_data(const RangeReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto RangeTypFormatter::get_avg_days(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTypFormatter::get_no_records_msg() const -> std::string {
  return config_->get_no_records_message();
}

void RangeTypFormatter::format_page_setup(
    std::stringstream& report_stream) const {
  report_stream << std::format(
            R"(#set page(margin: (top: {}cm, bottom: {}cm, left: {}cm, right: {}cm)))",
            config_->get_margin_top_cm(), config_->get_margin_bottom_cm(),
            config_->get_margin_left_cm(), config_->get_margin_right_cm())
     << "\n";
}

void RangeTypFormatter::format_header_content(
    std::stringstream& report_stream, const RangeReportData& data) const {
  std::string title =
      format_title_template(config_->get_title_template(), data);
  std::string title_line = std::format(
      R"(#text(font: "{}", size: {}pt)[= {}])", config_->get_title_font(),
      config_->get_report_title_font_size(), title);
  report_stream << title_line << "\n\n";

  if (data.actual_days > 0) {
    report_stream << std::format("+ *{}:* {}\n",
                                 config_->get_total_time_label(),
                      time_format_duration(data.total_duration,
                                            data.actual_days));
    report_stream << std::format("+ *{}:* {}\n",
                                 config_->get_actual_days_label(),
                                 data.actual_days);
  }
}

extern "C" {
  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto create_formatter(const char* config_toml)
      -> FormatterHandle {
    try {
      auto config_tbl = toml::parse(config_toml);
      auto typ_config = std::make_shared<RangeTypConfig>(config_tbl);
      auto formatter = std::make_unique<RangeTypFormatter>(typ_config);
      return static_cast<FormatterHandle>(formatter.release());
    } catch (...) {
      return nullptr;
    }
  }

  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
    if (handle != nullptr) {
      std::unique_ptr<RangeTypFormatter>{
          static_cast<RangeTypFormatter*>(handle)};
    }
  }

  static std::string report_buffer;

  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto format_report(FormatterHandle handle,
                                           const RangeReportData& data)
      -> const char* {
    if (handle != nullptr) {
      auto* formatter = static_cast<RangeTypFormatter*>(handle);
      report_buffer = formatter->format_report(data);
      return report_buffer.c_str();
    }
    return kEmptyReport;
  }
}
