#ifndef APPLICATION_DTO_REPORTING_REQUESTS_HPP_
#define APPLICATION_DTO_REPORTING_REQUESTS_HPP_

#include <optional>
#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"

namespace tracer_core::core::dto {

enum class TemporalSelectionKind {
  kSingleDay,
  kDateRange,
  kRecentDays,
};

enum class ReportDisplayMode {
  kDay,
  kWeek,
  kMonth,
  kYear,
  kRange,
  kRecent,
};

enum class ReportOperationKind {
  kQuery,
  kStructuredQuery,
  kTargets,
  kExport,
};

enum class ReportExportScope {
  kSingle,
  kAllMatching,
  kBatchRecentList,
};

struct PeriodBatchQueryRequest {
  std::vector<int> days_list;
  ReportFormat format = ReportFormat::kMarkdown;
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

struct TemporalReportQueryRequest {
  ReportDisplayMode display_mode = ReportDisplayMode::kDay;
  TemporalSelectionPayload selection;
  ReportFormat format = ReportFormat::kMarkdown;
};

struct TemporalStructuredReportQueryRequest {
  ReportDisplayMode display_mode = ReportDisplayMode::kDay;
  TemporalSelectionPayload selection;
};

struct TemporalReportTargetsRequest {
  ReportDisplayMode display_mode = ReportDisplayMode::kDay;
};

struct TemporalReportExportRequest {
  ReportDisplayMode display_mode = ReportDisplayMode::kDay;
  ReportExportScope export_scope = ReportExportScope::kSingle;
  ReportFormat format = ReportFormat::kMarkdown;
  std::optional<TemporalSelectionPayload> selection;
  std::vector<int> recent_days_list;
  std::string output_root_path;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_REPORTING_REQUESTS_HPP_
