// infra/insights/range/formatters/latex/range_tex_formatter_core.cpp
#include <memory>
#include <string>

#include "infra/insights/range/formatters/latex/range_tex_formatter.hpp"
#include "infra/insights/range/formatters/latex/range_tex_utils.hpp"

RangeTexConfig::RangeTexConfig(const RangeInsightsLabels& labels,
                               const FontConfig& fonts,
                               const LayoutConfig& layout)
    : RangeBaseConfig(labels), style_(fonts, layout) {}

RangeTexFormatter::RangeTexFormatter(std::shared_ptr<RangeTexConfig> config)
    : BaseTexFormatter(std::move(config)) {}

auto RangeTexFormatter::ValidateData(const RangeInsightsData& data) const
    -> std::string {
  if (!data.is_valid) {
    return config_->GetInvalidRangeMessage() + "\n";
  }
  return std::string{};
}

auto RangeTexFormatter::IsEmptyData(const RangeInsightsData& data) const
    -> bool {
  return data.actual_days == 0;
}

auto RangeTexFormatter::GetAvgDays(const RangeInsightsData& data) const -> int {
  return data.actual_days;
}

auto RangeTexFormatter::GetNoRecordsMsg() const -> std::string {
  return config_->GetNoRecordsMessage();
}

void RangeTexFormatter::FormatHeaderContent(
    std::string& insights_stream, const RangeInsightsData& data) const {
  RangeTexUtils::DisplaySummary(insights_stream, data, config_);
}
