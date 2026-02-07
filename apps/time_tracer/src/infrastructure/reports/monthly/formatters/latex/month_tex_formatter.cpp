// infrastructure/reports/monthly/formatters/latex/month_tex_formatter.cpp
#include "infrastructure/reports/monthly/formatters/latex/month_tex_formatter.hpp"

#include <toml++/toml.h>

#include <memory>

#include "infrastructure/reports/monthly/formatters/latex/month_tex_utils.hpp"

MonthTexFormatter::MonthTexFormatter(std::shared_ptr<MonthTexConfig> config)
    : BaseTexFormatter(config) {}

auto MonthTexFormatter::ValidateData(const MonthlyReportData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage() + "\n";
  }
  return "";
}

auto MonthTexFormatter::IsEmptyData(const MonthlyReportData& data) const
    -> bool {
  return data.total_duration <= 0;
}

auto MonthTexFormatter::GetAvgDays(const MonthlyReportData& data) const -> int {
  return data.actual_days;
}

auto MonthTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTexFormatter::FormatHeaderContent(
    std::stringstream& report_stream, const MonthlyReportData& data) const {
  MonthTexUtils::DisplayHeader(report_stream, data, config_);
}

namespace {
constexpr const char* kEmptyReport = "";
}  // namespace

// NOLINTBEGIN(readability-identifier-naming)
extern "C" {
__declspec(dllexport) FormatterHandle
CreateFormatter(const char* config_toml) {  // NOLINT
  try {
    // [FIX] 直接使用 parse 结果作为 table
    auto config_tbl = toml::parse(config_toml);

    // [FIX] 移除解引用
    auto tex_config = std::make_shared<MonthTexConfig>(config_tbl);
    auto formatter = std::make_unique<MonthTexFormatter>(tex_config);
    return static_cast<FormatterHandle>(formatter.release());
  } catch (...) {
    return nullptr;
  }
}
// NOLINTEND(readability-identifier-naming)

__declspec(dllexport) void DestroyFormatter(FormatterHandle handle) {  // NOLINT
  if (handle != nullptr) {
    std::unique_ptr<MonthTexFormatter>{static_cast<MonthTexFormatter*>(handle)};
  }
}

static std::string report_buffer;

__declspec(dllexport) auto FormatReport(
    FormatterHandle handle,
    const MonthlyReportData& data)  // NOLINT
    -> const char* {
  if (handle != nullptr) {
    auto* formatter = static_cast<MonthTexFormatter*>(handle);
    report_buffer = formatter->FormatReport(data);
    return report_buffer.c_str();
  }
  return kEmptyReport;
}
}
