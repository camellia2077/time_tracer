#ifndef APPLICATION_DTO_INSIGHTS_RESPONSES_HPP_
#define APPLICATION_DTO_INSIGHTS_RESPONSES_HPP_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "application/dto/insights_requests.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "domain/insights/models/daily_insights_data.hpp"
#include "domain/insights/models/period_insights_models.hpp"

namespace tracer_core::core::dto {

using TemporalInsightsDto = std::variant<DailyInsightsData, PeriodInsightsData>;

struct TemporalStructuredInsightsOutput {
  bool ok = true;
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
  TemporalSelectionKind selection_kind = TemporalSelectionKind::kSingleDay;
  TemporalInsightsDto insights = DailyInsightsData{};
  std::string error_message;
  ErrorContractFields error_contract;
};

struct StructuredPeriodBatchItem {
  int kDays = 0;
  bool ok = true;
  std::optional<PeriodInsightsData> insights;
  std::string error_message;
};

struct StructuredPeriodBatchOutput {
  bool ok = true;
  std::vector<StructuredPeriodBatchItem> items;
  std::string error_message;
  ErrorContractFields error_contract;
};

struct TemporalInsightsTargetsOutput {
  bool ok = true;
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
  std::vector<std::string> items;
  std::string error_message;
  ErrorContractFields error_contract;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_INSIGHTS_RESPONSES_HPP_
