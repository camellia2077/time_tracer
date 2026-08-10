// infra/insights/shared/interfaces/i_insights_formatter.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_INTERFACES_I_INSIGHTS_FORMATTER_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_INTERFACES_I_INSIGHTS_FORMATTER_H_

#include <string>

#include "domain/insights/models/daily_insights_data.hpp"
#include "domain/insights/models/period_insights_models.hpp"
#include "domain/insights/models/range_insights_data.hpp"

template <typename InsightsDataType>
class IInsightsFormatter {
 public:
  virtual ~IInsightsFormatter() = default;
  [[nodiscard]] virtual auto FormatInsights(const InsightsDataType& data) const
      -> std::string = 0;
};

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_INTERFACES_I_INSIGHTS_FORMATTER_H_
