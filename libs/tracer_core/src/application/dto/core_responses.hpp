// application/dto/core_responses.hpp
#ifndef APPLICATION_DTO_CORE_RESPONSES_H_
#define APPLICATION_DTO_CORE_RESPONSES_H_

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"

namespace tracer_core::core::dto {

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

struct TracerExchangeExportResult {
  bool ok = true;
  std::filesystem::path resolved_output_tracer_path;
  std::string source_root_name;
  std::uint64_t payload_file_count = 0;
  std::uint64_t converter_file_count = 0;
  bool manifest_included = false;
  std::string error_message;
};

struct TracerExchangeImportResult {
  bool ok = true;
  std::string source_root_name;
  std::uint64_t payload_file_count = 0;
  std::uint64_t replaced_month_count = 0;
  std::uint64_t preserved_month_count = 0;
  std::uint64_t rebuilt_month_count = 0;
  bool text_root_updated = false;
  bool config_applied = false;
  bool database_rebuilt = false;
  std::optional<std::filesystem::path> retained_failure_root;
  std::optional<std::filesystem::path> backup_retained_root;
  std::string backup_cleanup_error;
  std::string error_message;
};

struct TracerExchangeInspectOuterMetadata {
  std::uint8_t version = 0;
  std::uint8_t kdf_id = 0;
  std::uint8_t cipher_id = 0;
  std::uint8_t compression_id = 0;
  std::uint8_t compression_level = 0;
  std::uint32_t ops_limit = 0;
  std::uint32_t mem_limit_kib = 0;
  std::uint64_t plaintext_size = 0;
  std::uint64_t ciphertext_size = 0;
};

struct TracerExchangeInspectEntrySummary {
  std::string relative_path;
  bool present = false;
  std::uint64_t size_bytes = 0;
};

struct TracerExchangeInspectResult {
  bool ok = true;
  std::filesystem::path input_tracer_path;
  TracerExchangeInspectOuterMetadata outer_metadata{};
  std::string package_type;
  std::int64_t package_version = 0;
  std::string producer_platform;
  std::string producer_app;
  std::string created_at_utc;
  std::string source_root_name;
  std::uint64_t payload_file_count = 0;
  std::vector<TracerExchangeInspectEntrySummary> payload_entries;
  std::array<TracerExchangeInspectEntrySummary, 3> converter_entries{};
  std::string error_message;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_CORE_RESPONSES_H_
