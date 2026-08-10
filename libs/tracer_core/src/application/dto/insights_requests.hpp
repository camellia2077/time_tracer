#ifndef APPLICATION_DTO_INSIGHTS_REQUESTS_HPP_
#define APPLICATION_DTO_INSIGHTS_REQUESTS_HPP_

#include <optional>
#include <string>
#include <vector>

#include "domain/insights/types/insights_types.hpp"

namespace tracer_core::core::dto {

enum class TemporalSelectionKind {
  kSingleDay,
  kDateRange,
  kRecentDays,
};

enum class InsightsDisplayMode {
  kDay,
  kWeek,
  kMonth,
  kYear,
  kRange,
  kRecent,
};

enum class InsightsOperationKind {
  kQuery,
  kStructuredQuery,
  kTargets,
  kExport,
};

enum class InsightsExportScope {
  kSingle,
  kAllMatching,
  kBatchRecentList,
};

struct PeriodBatchQueryRequest {
  std::vector<int> days_list;
  InsightsFormat format = InsightsFormat::kMarkdown;
};

struct StructuredPeriodBatchQueryRequest {
  std::vector<int> kDays;
};

struct TemporalSelectionPayload {
  TemporalSelectionKind kind = TemporalSelectionKind::kSingleDay;
  std::string date;
  std::string start_date;
  std::string end_date;
  int days = 0;
  std::optional<std::string> anchor_date;
};

struct TemporalInsightsQueryRequest {
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
  TemporalSelectionPayload selection;
  InsightsFormat format = InsightsFormat::kMarkdown;
  std::string locale = "en";
};

struct TemporalStructuredInsightsQueryRequest {
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
  TemporalSelectionPayload selection;
};

struct TemporalInsightsTargetsRequest {
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
};

struct TemporalInsightsExportRequest {
  InsightsDisplayMode display_mode = InsightsDisplayMode::kDay;
  InsightsExportScope export_scope = InsightsExportScope::kSingle;
  InsightsFormat format = InsightsFormat::kMarkdown;
  std::optional<TemporalSelectionPayload> selection;
  std::vector<int> recent_days_list;
  std::string output_root_path;
  std::string locale = "en";
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_INSIGHTS_REQUESTS_HPP_
