// application/dto/core_requests.hpp
#ifndef APPLICATION_DTO_CORE_REQUESTS_H_
#define APPLICATION_DTO_CORE_REQUESTS_H_

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "domain/reports/types/report_types.hpp"
#include "domain/types/date_check_mode.hpp"
#include "domain/types/ingest_mode.hpp"

namespace tracer_core::core::dto {

enum class ReportQueryType {
  kDay,
  kMonth,
  kRecent,
  kRange,
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
  kActivitySuggest,
  kMappingNames,
  kReportChart,
  kTree,
};

enum class DataQueryOutputMode {
  kText,
  kSemanticJson,
};

enum class TracerExchangeSecurityLevel {
  kMin,
  kInteractive,
  kModerate,
  kHigh,
  kMax,
};

enum class TracerExchangeProgressControl {
  kContinue,
  kCancel,
};

struct TracerExchangeProgressSnapshot {
  std::filesystem::path input_root_path;
  std::filesystem::path output_root_path;
  std::filesystem::path current_input_path;
  std::filesystem::path current_output_path;
  std::string current_item;
  std::string current_group_label;
  std::size_t phase_index = 0;
  std::size_t phase_count = 0;
  std::size_t done_count = 0;
  std::size_t total_count = 0;
  std::size_t group_index = 0;
  std::size_t group_count = 0;
  std::size_t file_index_in_group = 0;
  std::size_t file_count_in_group = 0;
  std::size_t current_file_index = 0;
  std::size_t total_files = 0;
  std::uint64_t current_file_done_bytes = 0;
  std::uint64_t current_file_total_bytes = 0;
  std::uint64_t overall_done_bytes = 0;
  std::uint64_t overall_total_bytes = 0;
  std::uint64_t speed_bytes_per_sec = 0;
  std::uint64_t remaining_bytes = 0;
  std::uint64_t eta_seconds = 0;
  bool is_encrypt_operation = true;
  std::string phase;
};

using TracerExchangeProgressObserver = std::function<
    TracerExchangeProgressControl(const TracerExchangeProgressSnapshot&)>;

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
  IngestMode ingest_mode = IngestMode::kStandard;
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
  DataQueryOutputMode output_mode = DataQueryOutputMode::kText;
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
  std::optional<int> lookback_days;
  std::optional<std::string> activity_prefix;
  bool activity_score_by_duration = false;
  std::optional<std::string> tree_period;
  std::optional<std::string> tree_period_argument;
  std::optional<int> tree_max_depth;
  std::optional<std::string> root;
};

struct TreeQueryRequest {
  bool list_roots = false;
  std::string root_pattern;
  int max_depth = -1;
  std::optional<std::string> period;
  std::optional<std::string> period_argument;
  std::optional<std::string> root;
};

struct TracerExchangeExportRequest {
  std::filesystem::path input_text_root_path;
  std::filesystem::path requested_output_path;
  std::filesystem::path active_converter_main_config_path;
  DateCheckMode date_check_mode = DateCheckMode::kNone;
  std::string passphrase;
  std::string producer_platform;
  std::string producer_app;
  TracerExchangeSecurityLevel security_level =
      TracerExchangeSecurityLevel::kInteractive;
  TracerExchangeProgressObserver progress_observer{};
};

struct TracerExchangeImportRequest {
  std::filesystem::path input_tracer_path;
  std::filesystem::path active_text_root_path;
  std::filesystem::path active_converter_main_config_path;
  std::filesystem::path runtime_work_root;
  DateCheckMode date_check_mode = DateCheckMode::kContinuity;
  std::string passphrase;
  TracerExchangeProgressObserver progress_observer{};
};

struct TracerExchangeInspectRequest {
  std::filesystem::path input_tracer_path;
  std::string passphrase;
  TracerExchangeProgressObserver progress_observer{};
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_CORE_REQUESTS_H_
