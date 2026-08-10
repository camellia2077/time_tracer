// infra/insights/range/formatters/typst/range_typ_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_

#include <memory>

#include "domain/insights/models/range_insights_data.hpp"
#include "infra/insights/range/formatters/typst/range_typ_config.hpp"
#include "infra/insights/shared/formatters/templates/base_typ_formatter.hpp"

class RangeTypFormatter
    : public BaseTypFormatter<RangeInsightsData, RangeTypConfig> {
 public:
  explicit RangeTypFormatter(std::shared_ptr<RangeTypConfig> config);

 private:
  [[nodiscard]] auto ValidateData(const RangeInsightsData& data) const
      -> std::string override;
  [[nodiscard]] auto IsEmptyData(const RangeInsightsData& data) const
      -> bool override;
  [[nodiscard]] auto GetAvgDays(const RangeInsightsData& data) const
      -> int override;
  [[nodiscard]] auto GetNoRecordsMsg() const -> std::string override;
  void FormatPageSetup(std::string& insights_stream) const override;
  void FormatHeaderContent(std::string& insights_stream,
                           const RangeInsightsData& data) const override;
};

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_TYPST_RANGE_TYP_FORMATTER_H_
