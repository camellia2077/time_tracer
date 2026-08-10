// infra/insights/range/formatters/markdown/range_md_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_

#include <memory>
#include <string>

#include "domain/insights/models/range_insights_data.hpp"
#include "infra/insights/range/formatters/markdown/range_md_config.hpp"
#include "infra/insights/shared/formatters/templates/base_md_formatter.hpp"

class RangeMdFormatter
    : public BaseMdFormatter<RangeInsightsData, RangeMdConfig> {
 public:
  explicit RangeMdFormatter(std::shared_ptr<RangeMdConfig> config);

 private:
  [[nodiscard]] auto ValidateData(const RangeInsightsData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const RangeInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const RangeInsightsData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatHeaderContent(std::string& insights_stream,
                           const RangeInsightsData& data) const override;
};

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_FORMATTER_H_
