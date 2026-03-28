#ifndef APPLICATION_DTO_REPORTING_REQUESTS_HPP_
#define APPLICATION_DTO_REPORTING_REQUESTS_HPP_

#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"

namespace tracer_core::core::dto {

enum class ReportQueryType {
  kDay,
  kMonth,
  kRecent,
  kRange,
  kWeek,
  kYear,
};

enum class ReportTargetType {
  kDay,
  kMonth,
  kWeek,
  kYear,
};

enum class ReportExportType {
  kDay,
  kMonth,
  kRecent,
  kWeek,
  kYear,
  kAllDay,
  kAllMonth,
  kAllRecent,
  kAllWeek,
  kAllYear,
};

struct ReportQueryRequest {
  ReportQueryType type = ReportQueryType::kDay;
  std::string argument;
  ReportFormat format = ReportFormat::kMarkdown;
};

struct PeriodBatchQueryRequest {
  std::vector<int> days_list;
  ReportFormat format = ReportFormat::kMarkdown;
};

struct StructuredReportQueryRequest {
  ReportQueryType type = ReportQueryType::kDay;
  std::string argument;
};

struct StructuredPeriodBatchQueryRequest {
  std::vector<int> kDays;
};

struct ReportTargetsRequest {
  ReportTargetType type = ReportTargetType::kDay;
};

struct ReportExportRequest {
  ReportExportType type = ReportExportType::kDay;
  ReportFormat format = ReportFormat::kMarkdown;
  std::string argument;
  std::vector<int> recent_days_list;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_REPORTING_REQUESTS_HPP_
