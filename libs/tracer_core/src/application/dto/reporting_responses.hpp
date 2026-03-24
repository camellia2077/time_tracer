#ifndef APPLICATION_DTO_REPORTING_RESPONSES_HPP_
#define APPLICATION_DTO_REPORTING_RESPONSES_HPP_

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"

namespace tracer_core::core::dto {

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

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_REPORTING_RESPONSES_HPP_
