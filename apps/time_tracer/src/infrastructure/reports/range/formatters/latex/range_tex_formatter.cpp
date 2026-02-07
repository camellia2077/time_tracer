// infrastructure/reports/range/formatters/latex/range_tex_formatter.cpp
#include "infrastructure/reports/range/formatters/latex/range_tex_formatter.hpp"

#include <toml++/toml.h>

#include <memory>

#include "infrastructure/reports/range/formatters/latex/range_tex_utils.hpp"

RangeTexFormatter::RangeTexFormatter(std::shared_ptr<RangeTexConfig> config)
    : BaseTexFormatter(config) {}

auto RangeTexFormatter::ValidateData(const RangeReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage() + "\n";
  }
  return std::string{};
}

auto RangeTexFormatter::IsEmptyData(const RangeReportData& data) const -> bool {
  return data.actual_days == 0;
}

auto RangeTexFormatter::GetAvgDays(const RangeReportData& data) const -> int {
  return data.actual_days;
}

auto RangeTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTexFormatter::FormatHeaderContent(std::stringstream& report_stream,
                                            const RangeReportData& data) const {
  RangeTexUtils::DisplaySummary(report_stream, data, config_);
}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
// Public API: keep symbol name stable for dynamic loading.
__declspec(dllexport) auto CreateFormatter(const char* config_toml)
    -> FormatterHandle {  // NOLINT
  try {
    auto config_tbl = toml::parse(config_toml);
    auto tex_config = std::make_shared<RangeTexConfig>(config_tbl);
    auto formatter = std::make_unique<RangeTexFormatter>(tex_config);
    return static_cast<FormatterHandle>(formatter.release());
  } catch (...) {
    return nullptr;
  }
}
// NOLINTEND(readability-identifier-naming)

// Public API: keep symbol name stable for dynamic loading.
__declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {  // NOLINT
  if (handle != nullptr) {
    std::unique_ptr<RangeTexFormatter>{static_cast<RangeTexFormatter*>(handle)};
  }
}

static std::string report_buffer;

__declspec(dllexport) auto FormatReport(FormatterHandle handle,
                                        const RangeReportData& data) -> const
    char* {  // NOLINT
  if (handle != nullptr) {
    auto* formatter = static_cast<RangeTexFormatter*>(handle);
    report_buffer = formatter->FormatReport(data);
    return report_buffer.c_str();
  }
  return kEmptyReport;
}
}
