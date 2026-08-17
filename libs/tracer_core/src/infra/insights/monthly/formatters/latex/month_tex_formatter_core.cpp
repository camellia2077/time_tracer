// infra/insights/monthly/formatters/latex/month_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/monthly/formatters/latex/month_tex_formatter.hpp"
#include "infra/insights/monthly/formatters/latex/month_tex_utils.hpp"

MonthTexConfig::MonthTexConfig(const MonthlyTexConfig& config)
    : MonthBaseConfig(config.labels), style_(config.fonts, config.layout) {}

MonthTexFormatter::MonthTexFormatter(std::shared_ptr<MonthTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto MonthTexFormatter::ValidateData(const MonthlyInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidFormatMessage() + "\n";
  }
  return "";
}

auto MonthTexFormatter::IsEmptyData(const MonthlyInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto MonthTexFormatter::GetAvgDays(const MonthlyInsightsData& data) const
    -> int {
  return data.actual_days;
}

auto MonthTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void MonthTexFormatter::FormatHeaderContent(
    std::string& insights_stream, const MonthlyInsightsData& data) const {
  MonthTexUtils::DisplayHeader(insights_stream, data, config_);
}
