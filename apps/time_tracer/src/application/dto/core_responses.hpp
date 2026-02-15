// application/dto/core_responses.hpp
#ifndef APPLICATION_DTO_CORE_RESPONSES_H_
#define APPLICATION_DTO_CORE_RESPONSES_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"
#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"

namespace time_tracer::core::dto {

struct OperationAck {
  bool ok = true;
  std::string error_message;
};

struct TextOutput {
  bool ok = true;
  std::string content;
  std::string error_message;
};

enum class StructuredReportKind {
  kDay,
  kMonth,
  kRecent,
  kRange,
  kWeek,
  kYear,
};

using ReportDto =
    std::variant<DailyReportData, MonthlyReportData, PeriodReportData,
                 WeeklyReportData, YearlyReportData>;

struct StructuredReportOutput {
  bool ok = true;
  StructuredReportKind kind = StructuredReportKind::kDay;
  ReportDto report = DailyReportData{};
  std::string error_message;
};

struct StructuredPeriodBatchItem {
  int kDays = 0;
  bool ok = true;
  std::optional<PeriodReportData> report;
  std::string error_message;
};

struct StructuredPeriodBatchOutput {
  bool ok = true;
  std::vector<StructuredPeriodBatchItem> items;
  std::string error_message;
};

struct TreeQueryResponse {
  bool ok = true;
  bool found = true;
  std::vector<std::string> roots;
  std::vector<ProjectTreeNode> nodes;
  std::string error_message;
};

}  // namespace time_tracer::core::dto

#endif  // APPLICATION_DTO_CORE_RESPONSES_H_
