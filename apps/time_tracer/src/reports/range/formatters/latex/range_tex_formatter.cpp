// reports/range/formatters/latex/range_tex_formatter.cpp
#include "range_tex_formatter.hpp"

#include <memory>
#include <toml++/toml.h>

#include "reports/range/formatters/latex/range_tex_utils.hpp"

RangeTexFormatter::RangeTexFormatter(std::shared_ptr<RangeTexConfig> config)
    : BaseTexFormatter(config) {}

auto RangeTexFormatter::validate_data(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->get_invalid_range_message() + "\n";
  }
  return std::string{};
}

auto RangeTexFormatter::is_empty_data(const RangeReportData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto RangeTexFormatter::get_avg_days(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTexFormatter::get_no_records_msg() const -> std::string {
  return config_->get_no_records_message();
}

void RangeTexFormatter::format_header_content(
    std::stringstream& report_stream, const RangeReportData& data) const {
  RangeTexUtils::display_summary(report_stream, data, config_);
}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

extern "C" {
  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto create_formatter(const char* config_toml)
      -> FormatterHandle {
    try {
      auto config_tbl = toml::parse(config_toml);
      auto tex_config = std::make_shared<RangeTexConfig>(config_tbl);
      auto formatter = std::make_unique<RangeTexFormatter>(tex_config);
      return static_cast<FormatterHandle>(formatter.release());
    } catch (...) {
      return nullptr;
    }
  }

  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) void destroy_formatter(FormatterHandle handle) {
    if (handle != nullptr) {
      std::unique_ptr<RangeTexFormatter>{
          static_cast<RangeTexFormatter*>(handle)};
    }
  }

  static std::string report_buffer;

  // Public API: keep symbol name stable for dynamic loading.
  // NOLINTNEXTLINE(readability-identifier-naming)
  __declspec(dllexport) auto format_report(FormatterHandle handle,
                                           const RangeReportData& data)
      -> const char* {
    if (handle != nullptr) {
      auto* formatter = static_cast<RangeTexFormatter*>(handle);
      report_buffer = formatter->format_report(data);
      return report_buffer.c_str();
    }
    return kEmptyReport;
  }
}
