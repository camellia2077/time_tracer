// infra/insights/range/formatters/latex/range_tex_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_

#include <memory>

#include "domain/insights/models/range_insights_data.hpp"
#include "infra/insights/range/formatters/latex/range_tex_config.hpp"
#include "infra/insights/shared/formatters/templates/base_tex_formatter.hpp"

class RangeTexFormatter
    : public BaseTexFormatter<RangeInsightsData, RangeTexConfig> {
 public:
  explicit RangeTexFormatter(std::shared_ptr<RangeTexConfig> config);

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

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_LATEX_RANGE_TEX_FORMATTER_H_
