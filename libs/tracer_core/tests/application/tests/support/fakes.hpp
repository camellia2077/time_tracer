// application/tests/support/fakes.hpp
#ifndef APPLICATION_TESTS_SUPPORT_FAKES_H_
#define APPLICATION_TESTS_SUPPORT_FAKES_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/compat/insights/i_insights_handler.hpp"
#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/dto/pipeline_responses.hpp"
#include "application/ports/query/i_data_query_service.hpp"
#include "application/ports/insights/i_insights_dto_formatter.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "application/ports/exchange/i_tracer_exchange_service.hpp"
#include "application/aggregate_runtime/tracer_core_runtime.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/repositories/i_project_repository.hpp"
#include "domain/types/app_options.hpp"
#include "domain/types/ingest_mode.hpp"

namespace tracer_core::application::tests {

class FakePipelineWorkflow final
    : public tracer::core::application::pipeline::IPipelineWorkflow {
 public:
  bool fail_convert = false;
  bool fail_ingest = false;
  bool fail_import = false;
  bool fail_validate_structure = false;
  bool fail_validate_logic = false;
  bool fail_ingest_replace_all = false;
  bool fail_ingest_sync_status_query = false;
  bool fail_clear_ingest_sync_status = false;
  bool fail_record_activity_atomically = false;
  bool fail_default_txt_day_marker = false;
  bool fail_resolve_txt_day_block = false;
  bool fail_replace_txt_day_block = false;
  bool fail_convert_txt_activity_names = false;

  std::string last_converter_input;
  AppOptions last_converter_options;
  std::string last_ingest_input;
  std::string last_ingest_replace_all_input;
  DateCheckMode last_ingest_mode = DateCheckMode::kNone;
  DateCheckMode last_ingest_replace_all_mode = DateCheckMode::kNone;
  bool last_ingest_save_processed = false;
  bool last_ingest_replace_all_save_processed = false;
  IngestMode last_ingest_import_mode = IngestMode::kStandard;
  std::string last_import_path;
  std::string last_validate_structure_input;
  std::string last_validate_logic_input;
  DateCheckMode last_validate_logic_mode = DateCheckMode::kNone;
  tracer_core::core::dto::RecordActivityAtomicallyRequest
      last_record_activity_request;
  tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest
      last_update_activity_remark_request;
  tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest
      last_update_day_remark_request;
  tracer_core::core::dto::DefaultTxtDayMarkerRequest
      last_default_txt_day_marker_request;
  tracer_core::core::dto::ResolveTxtDayBlockRequest
      last_resolve_txt_day_block_request;
  tracer_core::core::dto::ReplaceTxtDayBlockRequest
      last_replace_txt_day_block_request;
  tracer_core::core::dto::ConvertTxtActivityNamesRequest
      last_convert_txt_activity_names_request;
  tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest
      last_replace_txt_canonical_activity_names_request;

  int convert_call_count = 0;
  int ingest_call_count = 0;
  int ingest_replace_all_call_count = 0;
  int import_call_count = 0;
  int validate_structure_call_count = 0;
  int validate_logic_call_count = 0;
  int ingest_sync_status_query_call_count = 0;
  int clear_ingest_sync_status_call_count = 0;
  int record_activity_atomically_call_count = 0;
  int update_activity_remark_atomically_call_count = 0;
  int update_day_remark_atomically_call_count = 0;
  int default_txt_day_marker_call_count = 0;
  int resolve_txt_day_block_call_count = 0;
  int replace_txt_day_block_call_count = 0;
  int convert_txt_activity_names_call_count = 0;
  int replace_txt_canonical_activity_names_call_count = 0;
  tracer_core::core::dto::IngestSyncStatusRequest
      last_ingest_sync_status_request;
  tracer_core::core::dto::IngestSyncStatusOutput ingest_sync_status_output = {
      .ok = true,
      .items = {},
      .error_message = "",
  };
  tracer_core::core::dto::RecordActivityAtomicallyResponse
      record_activity_atomically_response = {
          .ok = true,
          .message = "record: ok\nsync: ok",
          .operation_id = "fake-txn",
          .warnings = {},
          .rollback_failed = false,
          .retained_transaction_root = std::nullopt,
      };
  tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse
      update_activity_remark_atomically_response = {
          .ok = true,
          .message = "remark: ok\nsync: ok",
          .operation_id = "fake-remark-txn",
          .warnings = {},
          .rollback_failed = false,
          .retained_transaction_root = std::nullopt,
      };
  tracer_core::core::dto::DefaultTxtDayMarkerResponse
      default_txt_day_marker_response = {
          .ok = true,
          .normalized_day_marker = "0102",
          .error_message = "",
      };
  tracer_core::core::dto::ResolveTxtDayBlockResponse
      resolve_txt_day_block_response = {
          .ok = true,
          .normalized_day_marker = "0102",
          .found = true,
          .is_marker_valid = true,
          .can_save = true,
          .day_body = "0904study",
          .day_content_iso_date = std::string("2025-01-02"),
          .error_message = "",
      };
  tracer_core::core::dto::ReplaceTxtDayBlockResponse
      replace_txt_day_block_response = {
          .ok = true,
          .normalized_day_marker = "0102",
          .found = true,
          .is_marker_valid = true,
          .updated_content = "updated-content\n",
          .error_message = "",
      };

  auto RunConverter(const std::string& input_path, const AppOptions& options)
      -> void override;
  auto RunDatabaseImport(const std::string& processed_path_str)
      -> void override;
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> void override;
  auto RunIngest(const std::string& source_path, DateCheckMode date_check_mode,
                 bool save_processed, IngestMode ingest_mode) -> void override;
  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest& request)
      -> tracer_core::core::dto::IngestSyncStatusOutput override;
  auto ClearIngestSyncStatus() -> void override;
  auto RunIngestReplacingAll(const std::string& source_path,
                             DateCheckMode date_check_mode, bool save_processed)
      -> void override;
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;
  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override;
  auto RunUpdateActivityRemarkAtomically(
      const tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest&
          request)
      -> tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse
      override;
  auto RunUpdateDayRemarkAtomically(
      const tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest& request)
      -> tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse override;
  auto RunDefaultTxtDayMarker(
      const tracer_core::core::dto::DefaultTxtDayMarkerRequest& request)
      -> tracer_core::core::dto::DefaultTxtDayMarkerResponse override;
  auto RunResolveTxtDayBlock(
      const tracer_core::core::dto::ResolveTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ResolveTxtDayBlockResponse override;
  auto RunReplaceTxtDayBlock(
      const tracer_core::core::dto::ReplaceTxtDayBlockRequest& request)
      -> tracer_core::core::dto::ReplaceTxtDayBlockResponse override;
  auto RunConvertTxtActivityNames(
      const tracer_core::core::dto::ConvertTxtActivityNamesRequest& request)
      -> tracer_core::core::dto::ConvertTxtActivityNamesResponse override;
  auto RunReplaceTxtCanonicalActivityNames(
      const tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse
      override;
  auto RunReplaceTxtAliasActivityNames(
      const tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest&
          request)
      -> tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse override;
  auto InstallActiveConverterConfig(
      const tracer::core::application::pipeline::
          ActiveConverterConfigInstallRequest& request) -> void override;
};

class FakeInsightsHandler final : public IInsightsHandler {
 public:
  bool fail_query = false;
  bool fail_period_batch_query = false;
  bool fail_target_not_found = false;

  std::string daily_query_result = "daily";
  std::string monthly_query_result = "monthly";
  std::string weekly_query_result = "weekly";
  std::string yearly_query_result = "yearly";
  std::string recent_query_result = "recent";
  std::string period_batch_result = "period-batch";

  auto RunDailyQuery(std::string_view date, InsightsFormat format)
      -> std::string override;
  auto RunMonthlyQuery(std::string_view month, InsightsFormat format)
      -> std::string override;
  auto RunPeriodQuery(int days, InsightsFormat format) -> std::string override;
  auto RunWeeklyQuery(std::string_view iso_week, InsightsFormat format)
      -> std::string override;
  auto RunYearlyQuery(std::string_view year, InsightsFormat format)
      -> std::string override;
  auto RunPeriodQueries(const std::vector<int>& days_list, InsightsFormat format)
      -> std::string override;
};

class FakeInsightsDataQueryService final
    : public tracer_core::application::ports::IInsightsDataQueryService {
 public:
  bool fail_list_targets = false;
  bool fail_target_not_found = false;
  std::vector<InsightsStatusValue> period_statuses;

  std::vector<std::string> daily_targets = {"2026-01-03", "2026-01-04"};
  std::vector<std::string> monthly_targets = {"2026-01", "2026-02"};
  std::vector<std::string> weekly_targets = {"2026-W01", "2026-W02"};
  std::vector<std::string> yearly_targets = {"2025", "2026"};

  auto QueryDaily(std::string_view date) -> DailyInsightsData override;
  auto QueryMonthly(std::string_view month) -> MonthlyInsightsData override;
  auto QueryPeriod(int days) -> PeriodInsightsData override;
  auto QueryRange(std::string_view start_date, std::string_view end_date)
      -> PeriodInsightsData override;
  auto QueryWeekly(std::string_view iso_week) -> WeeklyInsightsData override;
  auto QueryYearly(std::string_view year) -> YearlyInsightsData override;

  auto ListDailyTargets() -> std::vector<std::string> override;
  auto ListMonthlyTargets() -> std::vector<std::string> override;
  auto ListWeeklyTargets() -> std::vector<std::string> override;
  auto ListYearlyTargets() -> std::vector<std::string> override;

  auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodInsightsData> override;
  auto QueryAllDaily() -> std::map<std::string, DailyInsightsData> override;
  auto QueryAllMonthly() -> std::map<std::string, MonthlyInsightsData> override;
  auto QueryAllWeekly() -> std::map<std::string, WeeklyInsightsData> override;
  auto QueryAllYearly() -> std::map<std::string, YearlyInsightsData> override;
};

class FakeInsightsDtoFormatter final
    : public tracer_core::application::ports::IInsightsDtoFormatter {
 public:
  auto FormatDaily(const DailyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatMonthly(const MonthlyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatPeriod(const PeriodInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatWeekly(const WeeklyInsightsData& insights, InsightsFormat format)
      -> std::string override;
  auto FormatYearly(const YearlyInsightsData& insights, InsightsFormat format)
      -> std::string override;
};

class FakeDataQueryService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  bool fail_query = false;
  tracer_core::core::dto::TextOutput response = {
      .ok = true, .content = "data-query-result", .error_message = ""};
  tracer_core::core::dto::DataQueryRequest last_request;
  int call_count = 0;

  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;
};

class FakeProjectRepository final : public IProjectRepository {
 public:
  bool fail_get_all_projects = false;
  int get_all_projects_call_count = 0;
  std::vector<ProjectEntity> projects = {
      {.id = 1, .parent_id = std::nullopt, .name = "root"}};

  auto GetAllProjects() -> std::vector<ProjectEntity> override;
};

class FakeTracerExchangeService final
    : public tracer_core::application::ports::ITracerExchangeService {
 public:
  auto BuildExportContent(
      const tracer_core::core::dto::TracerExchangeContentRequest& request)
      -> tracer_core::core::dto::TracerExchangeContentResult override;
  auto EncodeExportContent(
      const tracer_core::core::dto::TracerExchangeExportContent& content)
      -> tracer_core::core::dto::TracerExchangeContentEncodingResult override;
  bool throw_on_export = false;
  bool throw_on_import = false;
  bool throw_on_inspect = false;

  tracer_core::core::dto::TracerExchangeExportResult export_result{
      .ok = true,
      .resolved_output_tracer_path = "out/sample.zip",
      .source_root_name = "data",
      .payload_file_count = 2,
      .error_message = "",
  };
  tracer_core::core::dto::TracerExchangeImportResult import_result{
      .ok = true,
      .source_root_name = "data",
      .payload_file_count = 2,
      .replaced_month_count = 2,
      .preserved_month_count = 1,
      .rebuilt_month_count = 3,
      .text_root_updated = true,
      .config_applied = true,
      .database_rebuilt = true,
      .error_message = "",
  };
  tracer_core::core::dto::TracerExchangeInspectResult inspect_result{
      .ok = true,
      .input_tracer_path = "out/sample.zip",
      .package_type = "tracer_exchange",
      .package_version = 6,
      .source_root_name = "data",
      .payload_file_count = 2,
      .error_message = "",
  };

  tracer_core::core::dto::TracerExchangeExportContent content_result{
      .manifest = {},
      .entries = {},
  };
  tracer_core::core::dto::TracerExchangeContentEncodingResult encoding_result{
      .ok = true,
      .content = {},
      .error_message = "",
  };

  tracer_core::core::dto::TracerExchangeContentRequest last_content_request;
  tracer_core::core::dto::TracerExchangeExportContent last_encoding_content;
  tracer_core::core::dto::TracerExchangeExportRequest last_export_request;
  tracer_core::core::dto::TracerExchangeImportRequest last_import_request;
  tracer_core::core::dto::TracerExchangeInspectRequest last_inspect_request;
  int content_call_count = 0;
  int encoding_call_count = 0;
  int export_call_count = 0;
  int import_call_count = 0;
  int inspect_call_count = 0;

  auto RunExport(
      const tracer_core::core::dto::TracerExchangeExportRequest& request)
      -> tracer_core::core::dto::TracerExchangeExportResult override;

  auto RunImport(
      const tracer_core::core::dto::TracerExchangeImportRequest& request)
      -> tracer_core::core::dto::TracerExchangeImportResult override;

  auto RunInspect(
      const tracer_core::core::dto::TracerExchangeInspectRequest& request)
      -> tracer_core::core::dto::TracerExchangeInspectResult override;
};

auto BuildRuntimeApi(
    FakePipelineWorkflow& pipeline_workflow, FakeInsightsHandler& insights_handler,
    const std::shared_ptr<FakeProjectRepository>& repository,
    const std::shared_ptr<FakeDataQueryService>& data_query,
    const std::shared_ptr<FakeTracerExchangeService>& tracer_exchange_service =
        nullptr,
    const std::shared_ptr<FakeInsightsDataQueryService>&
        insights_data_query_service = nullptr) -> TracerCoreRuntime;

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_SUPPORT_FAKES_H_
