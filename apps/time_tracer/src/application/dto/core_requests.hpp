// application/dto/core_requests.hpp
#ifndef APPLICATION_DTO_CORE_REQUESTS_H_
#define APPLICATION_DTO_CORE_REQUESTS_H_

#include <optional>
#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"
#include "domain/types/date_check_mode.hpp"

namespace time_tracer::core::dto {

enum class ReportQueryType {
  kDay,
  kMonth,
  kRecent,
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

enum class DataQueryAction {
  kYears,
  kMonths,
  kDays,
  kDaysDuration,
  kDaysStats,
  kSearch,
};

struct ConvertRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;
  bool validate_logic = true;
  bool validate_structure = true;
};

struct IngestRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool save_processed_output = false;
};

struct ImportRequest {
  std::string processed_path;
};

struct ValidateStructureRequest {
  std::string input_path;
};

struct ValidateLogicRequest {
  std::string input_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
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

struct ReportExportRequest {
  ReportExportType type = ReportExportType::kDay;
  ReportFormat format = ReportFormat::kMarkdown;
  std::string argument;
  std::vector<int> recent_days_list;
};

struct DataQueryRequest {
  DataQueryAction action = DataQueryAction::kYears;
  std::optional<int> year;
  std::optional<int> month;
  std::optional<std::string> from_date;
  std::optional<std::string> to_date;
  std::optional<std::string> remark;
  std::optional<std::string> day_remark;
  std::optional<std::string> project;
  std::optional<int> exercise;
  std::optional<int> status;
  bool overnight = false;
  bool reverse = false;
  std::optional<int> limit;
  std::optional<int> top_n;
};

struct TreeQueryRequest {
  bool list_roots = false;
  std::string root_pattern;
  int max_depth = -1;
};

}  // namespace time_tracer::core::dto

#endif  // APPLICATION_DTO_CORE_REQUESTS_H_
