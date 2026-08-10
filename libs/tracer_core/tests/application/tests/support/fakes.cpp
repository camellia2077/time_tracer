// application/tests/support/fakes.cpp
#include "application/tests/support/fakes.hpp"

#include <stdexcept>

#include "application/use_cases/pipeline_api.hpp"
#include "application/use_cases/query_api.hpp"
#include "application/use_cases/insights_api.hpp"
#include "application/use_cases/tracer_exchange_api.hpp"
#include "shared/types/insights_errors.hpp"

namespace tracer_core::application::tests {

auto FakePipelineWorkflow::RunConverter(const std::string& input_path,
                                        const AppOptions& options) -> void {
  ++convert_call_count;
  last_converter_input = input_path;
  last_converter_options = options;
  if (fail_convert) {
    throw std::runtime_error("convert failed");
  }
}

auto FakePipelineWorkflow::RunDatabaseImport(
    const std::string& processed_path_str) -> void {
  ++import_call_count;
  last_import_path = processed_path_str;
  if (fail_import) {
    throw std::runtime_error("import failed");
  }
}

auto FakePipelineWorkflow::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& /*data_map*/) -> void {}

auto FakePipelineWorkflow::RunIngest(const std::string& source_path,
                                     DateCheckMode date_check_mode,
                                     bool save_processed,
                                     IngestMode ingest_mode) -> void {
  ++ingest_call_count;
  last_ingest_input = source_path;
  last_ingest_mode = date_check_mode;
  last_ingest_save_processed = save_processed;
  last_ingest_import_mode = ingest_mode;
  if (fail_ingest) {
    throw std::runtime_error("ingest failed");
  }
}

auto FakePipelineWorkflow::RunIngestSyncStatusQuery(
    const tracer_core::core::dto::IngestSyncStatusRequest& request)
    -> tracer_core::core::dto::IngestSyncStatusOutput {
  ++ingest_sync_status_query_call_count;
  last_ingest_sync_status_request = request;
  if (fail_ingest_sync_status_query) {
    throw std::runtime_error("ingest sync status query failed");
  }
  return ingest_sync_status_output;
}

auto FakePipelineWorkflow::ClearIngestSyncStatus() -> void {
  ++clear_ingest_sync_status_call_count;
  if (fail_clear_ingest_sync_status) {
    throw std::runtime_error("clear ingest sync status failed");
  }
}

auto FakePipelineWorkflow::RunIngestReplacingAll(const std::string& source_path,
                                                 DateCheckMode date_check_mode,
                                                 bool save_processed) -> void {
  ++ingest_replace_all_call_count;
  last_ingest_replace_all_input = source_path;
  last_ingest_replace_all_mode = date_check_mode;
  last_ingest_replace_all_save_processed = save_processed;
  if (fail_ingest_replace_all) {
    throw std::runtime_error("replace-all ingest failed");
  }
}

auto FakePipelineWorkflow::RunValidateStructure(const std::string& source_path)
    -> void {
  ++validate_structure_call_count;
  last_validate_structure_input = source_path;
  if (fail_validate_structure) {
    throw std::runtime_error("validate structure failed");
  }
}

auto FakePipelineWorkflow::RunValidateLogic(const std::string& source_path,
                                            DateCheckMode date_check_mode)
    -> void {
  ++validate_logic_call_count;
  last_validate_logic_input = source_path;
  last_validate_logic_mode = date_check_mode;
  if (fail_validate_logic) {
    throw std::runtime_error("validate logic failed");
  }
}

auto FakePipelineWorkflow::RunRecordActivityAtomically(
    const tracer_core::core::dto::RecordActivityAtomicallyRequest& request)
    -> tracer_core::core::dto::RecordActivityAtomicallyResponse {
  ++record_activity_atomically_call_count;
  last_record_activity_request = request;
  if (fail_record_activity_atomically) {
    throw std::runtime_error("record activity atomically failed");
  }
  return record_activity_atomically_response;
}

auto FakePipelineWorkflow::RunDefaultTxtDayMarker(
    const tracer_core::core::dto::DefaultTxtDayMarkerRequest& request)
    -> tracer_core::core::dto::DefaultTxtDayMarkerResponse {
  ++default_txt_day_marker_call_count;
  last_default_txt_day_marker_request = request;
  if (fail_default_txt_day_marker) {
    throw std::runtime_error("default txt day marker failed");
  }
  return default_txt_day_marker_response;
}

auto FakePipelineWorkflow::RunResolveTxtDayBlock(
    const tracer_core::core::dto::ResolveTxtDayBlockRequest& request)
    -> tracer_core::core::dto::ResolveTxtDayBlockResponse {
  ++resolve_txt_day_block_call_count;
  last_resolve_txt_day_block_request = request;
  if (fail_resolve_txt_day_block) {
    throw std::runtime_error("resolve txt day block failed");
  }
  return resolve_txt_day_block_response;
}

auto FakePipelineWorkflow::RunReplaceTxtDayBlock(
    const tracer_core::core::dto::ReplaceTxtDayBlockRequest& request)
    -> tracer_core::core::dto::ReplaceTxtDayBlockResponse {
  ++replace_txt_day_block_call_count;
  last_replace_txt_day_block_request = request;
  if (fail_replace_txt_day_block) {
    throw std::runtime_error("replace txt day block failed");
  }
  return replace_txt_day_block_response;
}

auto FakePipelineWorkflow::RunUpdateActivityRemarkAtomically(
    const tracer_core::core::dto::UpdateActivityRemarkAtomicallyRequest&
        request)
    -> tracer_core::core::dto::UpdateActivityRemarkAtomicallyResponse {
  ++update_activity_remark_atomically_call_count;
  last_update_activity_remark_request = request;
  return update_activity_remark_atomically_response;
}

auto FakePipelineWorkflow::RunUpdateDayRemarkAtomically(
    const tracer_core::core::dto::UpdateDayRemarkAtomicallyRequest& request)
    -> tracer_core::core::dto::UpdateDayRemarkAtomicallyResponse {
  ++update_day_remark_atomically_call_count;
  last_update_day_remark_request = request;
  return {.ok = true,
          .message = "day remark: ok\nsync: ok",
          .operation_id = "fake-day-remark-txn"};
}

auto FakePipelineWorkflow::RunConvertTxtActivityNames(
    const tracer_core::core::dto::ConvertTxtActivityNamesRequest& request)
    -> tracer_core::core::dto::ConvertTxtActivityNamesResponse {
  ++convert_txt_activity_names_call_count;
  last_convert_txt_activity_names_request = request;
  if (fail_convert_txt_activity_names) {
    throw std::runtime_error("convert txt activity names failed");
  }
  return {
      .ok = true, .converted_content = request.content, .error_message = ""};
}

auto FakePipelineWorkflow::RunReplaceTxtCanonicalActivityNames(
    const tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesRequest&
        request)
    -> tracer_core::core::dto::ReplaceTxtCanonicalActivityNamesResponse {
  ++replace_txt_canonical_activity_names_call_count;
  last_replace_txt_canonical_activity_names_request = request;
  return {.ok = true, .updated_content = request.content, .error_message = ""};
}

auto FakePipelineWorkflow::RunReplaceTxtAliasActivityNames(
    const tracer_core::core::dto::ReplaceTxtAliasActivityNamesRequest& request)
    -> tracer_core::core::dto::ReplaceTxtAliasActivityNamesResponse {
  return {.ok = true, .updated_content = request.content, .error_message = ""};
}

auto FakePipelineWorkflow::InstallActiveConverterConfig(
    const tracer::core::application::pipeline::
        ActiveConverterConfigInstallRequest& /*request*/) -> void {}

auto FakeInsightsHandler::RunDailyQuery(std::string_view /*date*/,
                                      InsightsFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("day", "missing-day");
  }
  if (fail_query) {
    throw std::runtime_error("daily query failed");
  }
  return daily_query_result;
}

auto FakeInsightsHandler::RunMonthlyQuery(std::string_view /*month*/,
                                        InsightsFormat /*format*/)
    -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("month",
                                                         "missing-month");
  }
  if (fail_query) {
    throw std::runtime_error("monthly query failed");
  }
  return monthly_query_result;
}

auto FakeInsightsHandler::RunPeriodQuery(int /*days*/, InsightsFormat /*format*/)
    -> std::string {
  if (fail_query) {
    throw std::runtime_error("recent query failed");
  }
  return recent_query_result;
}

auto FakeInsightsHandler::RunWeeklyQuery(std::string_view /*iso_week*/,
                                       InsightsFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("week",
                                                         "missing-week");
  }
  if (fail_query) {
    throw std::runtime_error("weekly query failed");
  }
  return weekly_query_result;
}

auto FakeInsightsHandler::RunYearlyQuery(std::string_view /*year*/,
                                       InsightsFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("year",
                                                         "missing-year");
  }
  if (fail_query) {
    throw std::runtime_error("yearly query failed");
  }
  return yearly_query_result;
}

auto FakeInsightsHandler::RunPeriodQueries(const std::vector<int>& /*days_list*/,
                                         InsightsFormat /*format*/)
    -> std::string {
  if (fail_period_batch_query) {
    throw std::runtime_error("period-batch query failed");
  }
  return period_batch_result;
}

auto FakeInsightsDataQueryService::QueryDaily(std::string_view date)
    -> DailyInsightsData {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("day", date);
  }
  DailyInsightsData insights;
  insights.date = std::string(date);
  return insights;
}

auto FakeInsightsDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyInsightsData {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("month", month);
  }
  MonthlyInsightsData insights;
  insights.range_label = std::string(month);
  return insights;
}

auto FakeInsightsDataQueryService::QueryPeriod(int days) -> PeriodInsightsData {
  PeriodInsightsData insights;
  insights.requested_days = days;
  return insights;
}

auto FakeInsightsDataQueryService::QueryRange(std::string_view start_date,
                                            std::string_view end_date)
    -> PeriodInsightsData {
  PeriodInsightsData insights;
  insights.start_date = std::string(start_date);
  insights.end_date = std::string(end_date);
  return insights;
}

auto FakeInsightsDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyInsightsData {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("week", iso_week);
  }
  WeeklyInsightsData insights;
  insights.range_label = std::string(iso_week);
  return insights;
}

auto FakeInsightsDataQueryService::QueryYearly(std::string_view year)
    -> YearlyInsightsData {
  if (fail_target_not_found) {
    throw tracer_core::common::InsightsTargetNotFoundError("year", year);
  }
  YearlyInsightsData insights;
  insights.range_label = std::string(year);
  return insights;
}

auto FakeInsightsDataQueryService::ListDailyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("daily target listing failed");
  }
  return daily_targets;
}

auto FakeInsightsDataQueryService::ListMonthlyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("monthly target listing failed");
  }
  return monthly_targets;
}

auto FakeInsightsDataQueryService::ListWeeklyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("weekly target listing failed");
  }
  return weekly_targets;
}

auto FakeInsightsDataQueryService::ListYearlyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("yearly target listing failed");
  }
  return yearly_targets;
}

auto FakeInsightsDataQueryService::QueryPeriodBatch(
    const std::vector<int>& /*days_list*/) -> std::map<int, PeriodInsightsData> {
  return {};
}

auto FakeInsightsDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyInsightsData> {
  return {};
}

auto FakeInsightsDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyInsightsData> {
  return {};
}

auto FakeInsightsDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyInsightsData> {
  return {};
}

auto FakeInsightsDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyInsightsData> {
  return {};
}

auto FakeInsightsDtoFormatter::FormatDaily(const DailyInsightsData& insights,
                                         InsightsFormat /*format*/)
    -> std::string {
  return "daily:" + insights.date;
}

auto FakeInsightsDtoFormatter::FormatMonthly(const MonthlyInsightsData& insights,
                                           InsightsFormat /*format*/)
    -> std::string {
  return "month:" + insights.range_label;
}

auto FakeInsightsDtoFormatter::FormatPeriod(const PeriodInsightsData& insights,
                                          InsightsFormat /*format*/)
    -> std::string {
  return "period:" + insights.start_date + "|" + insights.end_date;
}

auto FakeInsightsDtoFormatter::FormatWeekly(const WeeklyInsightsData& insights,
                                          InsightsFormat /*format*/)
    -> std::string {
  return "week:" + insights.range_label;
}

auto FakeInsightsDtoFormatter::FormatYearly(const YearlyInsightsData& insights,
                                          InsightsFormat /*format*/)
    -> std::string {
  return "year:" + insights.range_label;
}

auto FakeDataQueryService::RunDataQuery(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> tracer_core::core::dto::TextOutput {
  ++call_count;
  last_request = request;
  if (fail_query) {
    throw std::runtime_error("data query failed");
  }
  return response;
}

auto FakeProjectRepository::GetAllProjects() -> std::vector<ProjectEntity> {
  ++get_all_projects_call_count;
  if (fail_get_all_projects) {
    throw std::runtime_error("project repository failed");
  }
  return projects;
}

auto FakeTracerExchangeService::BuildExportContent(
    const tracer_core::core::dto::TracerExchangeContentRequest& request)
    -> tracer_core::core::dto::TracerExchangeContentResult {
  ++content_call_count;
  last_content_request = request;
  return {.ok = true, .content = content_result, .error_message = {}};
}

auto FakeTracerExchangeService::EncodeExportContent(
    const tracer_core::core::dto::TracerExchangeExportContent& content)
    -> tracer_core::core::dto::TracerExchangeContentEncodingResult {
  ++encoding_call_count;
  last_encoding_content = content;
  return encoding_result;
}

auto FakeTracerExchangeService::RunExport(
    const tracer_core::core::dto::TracerExchangeExportRequest& request)
    -> tracer_core::core::dto::TracerExchangeExportResult {
  ++export_call_count;
  last_export_request = request;
  if (throw_on_export) {
    throw std::runtime_error("exchange export failed");
  }
  return export_result;
}

auto FakeTracerExchangeService::RunImport(
    const tracer_core::core::dto::TracerExchangeImportRequest& request)
    -> tracer_core::core::dto::TracerExchangeImportResult {
  ++import_call_count;
  last_import_request = request;
  if (throw_on_import) {
    throw std::runtime_error("exchange import failed");
  }
  return import_result;
}

auto FakeTracerExchangeService::RunInspect(
    const tracer_core::core::dto::TracerExchangeInspectRequest& request)
    -> tracer_core::core::dto::TracerExchangeInspectResult {
  ++inspect_call_count;
  last_inspect_request = request;
  if (throw_on_inspect) {
    throw std::runtime_error("exchange inspect failed");
  }
  return inspect_result;
}

auto BuildRuntimeApi(
    FakePipelineWorkflow& pipeline_workflow, FakeInsightsHandler& insights_handler,
    const std::shared_ptr<FakeProjectRepository>& repository,
    const std::shared_ptr<FakeDataQueryService>& data_query,
    const std::shared_ptr<FakeTracerExchangeService>& tracer_exchange_service,
    const std::shared_ptr<FakeInsightsDataQueryService>&
        insights_data_query_service) -> TracerCoreRuntime {
  auto pipeline_api = std::make_shared<PipelineApi>(pipeline_workflow);
  auto query_api = std::make_shared<QueryApi>(repository, data_query);
  auto insights_formatter = std::make_shared<FakeInsightsDtoFormatter>();
  auto insights_api = std::make_shared<InsightsApi>(
      insights_handler, insights_data_query_service, insights_formatter);
  auto tracer_exchange_api =
      std::make_shared<TracerExchangeApi>(tracer_exchange_service);
  return {std::move(pipeline_api), std::move(query_api), std::move(insights_api),
          std::move(tracer_exchange_api)};
}

}  // namespace tracer_core::application::tests
