// application/tests/support/fakes.cpp
#include "application/tests/support/fakes.hpp"

#include <stdexcept>

#include "application/use_cases/pipeline_api.hpp"
#include "application/use_cases/query_api.hpp"
#include "application/use_cases/report_api.hpp"
#include "application/use_cases/tracer_exchange_api.hpp"
#include "shared/types/reporting_errors.hpp"

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

auto FakePipelineWorkflow::InstallActiveConverterConfig(
    const tracer::core::application::pipeline::
        ActiveConverterConfigInstallRequest& /*request*/) -> void {}

auto FakeReportHandler::RunDailyQuery(std::string_view /*date*/,
                                      ReportFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("day", "missing-day");
  }
  if (fail_query) {
    throw std::runtime_error("daily query failed");
  }
  return daily_query_result;
}

auto FakeReportHandler::RunMonthlyQuery(std::string_view /*month*/,
                                        ReportFormat /*format*/)
    -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("month",
                                                         "missing-month");
  }
  if (fail_query) {
    throw std::runtime_error("monthly query failed");
  }
  return monthly_query_result;
}

auto FakeReportHandler::RunPeriodQuery(int /*days*/, ReportFormat /*format*/)
    -> std::string {
  if (fail_query) {
    throw std::runtime_error("recent query failed");
  }
  return recent_query_result;
}

auto FakeReportHandler::RunWeeklyQuery(std::string_view /*iso_week*/,
                                       ReportFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("week", "missing-week");
  }
  if (fail_query) {
    throw std::runtime_error("weekly query failed");
  }
  return weekly_query_result;
}

auto FakeReportHandler::RunYearlyQuery(std::string_view /*year*/,
                                       ReportFormat /*format*/) -> std::string {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("year", "missing-year");
  }
  if (fail_query) {
    throw std::runtime_error("yearly query failed");
  }
  return yearly_query_result;
}

auto FakeReportHandler::RunPeriodQueries(const std::vector<int>& /*days_list*/,
                                         ReportFormat /*format*/)
    -> std::string {
  if (fail_period_batch_query) {
    throw std::runtime_error("period-batch query failed");
  }
  return period_batch_result;
}

auto FakeReportDataQueryService::QueryDaily(std::string_view date)
    -> DailyReportData {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("day", date);
  }
  DailyReportData report;
  report.date = std::string(date);
  return report;
}

auto FakeReportDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyReportData {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("month", month);
  }
  MonthlyReportData report;
  report.range_label = std::string(month);
  return report;
}

auto FakeReportDataQueryService::QueryPeriod(int days) -> PeriodReportData {
  PeriodReportData report;
  report.requested_days = days;
  return report;
}

auto FakeReportDataQueryService::QueryRange(std::string_view start_date,
                                            std::string_view end_date)
    -> PeriodReportData {
  PeriodReportData report;
  report.start_date = std::string(start_date);
  report.end_date = std::string(end_date);
  return report;
}

auto FakeReportDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyReportData {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("week", iso_week);
  }
  WeeklyReportData report;
  report.range_label = std::string(iso_week);
  return report;
}

auto FakeReportDataQueryService::QueryYearly(std::string_view year)
    -> YearlyReportData {
  if (fail_target_not_found) {
    throw tracer_core::common::ReportTargetNotFoundError("year", year);
  }
  YearlyReportData report;
  report.range_label = std::string(year);
  return report;
}

auto FakeReportDataQueryService::ListDailyTargets() -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("daily target listing failed");
  }
  return daily_targets;
}

auto FakeReportDataQueryService::ListMonthlyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("monthly target listing failed");
  }
  return monthly_targets;
}

auto FakeReportDataQueryService::ListWeeklyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("weekly target listing failed");
  }
  return weekly_targets;
}

auto FakeReportDataQueryService::ListYearlyTargets()
    -> std::vector<std::string> {
  if (fail_list_targets) {
    throw std::runtime_error("yearly target listing failed");
  }
  return yearly_targets;
}

auto FakeReportDataQueryService::QueryPeriodBatch(
    const std::vector<int>& /*days_list*/) -> std::map<int, PeriodReportData> {
  return {};
}

auto FakeReportDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyReportData> {
  return {};
}

auto FakeReportDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyReportData> {
  return {};
}

auto FakeReportDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyReportData> {
  return {};
}

auto FakeReportDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyReportData> {
  return {};
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

auto FakeTracerExchangeService::RunUnpack(
    const tracer_core::core::dto::TracerExchangeUnpackRequest& request)
    -> tracer_core::core::dto::TracerExchangeUnpackResult {
  ++unpack_call_count;
  last_unpack_request = request;
  if (throw_on_unpack) {
    throw std::runtime_error("unpack");
  }
  return unpack_result;
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
    FakePipelineWorkflow& pipeline_workflow, FakeReportHandler& report_handler,
    const std::shared_ptr<FakeProjectRepository>& repository,
    const std::shared_ptr<FakeDataQueryService>& data_query,
    const std::shared_ptr<FakeTracerExchangeService>& tracer_exchange_service,
    const std::shared_ptr<FakeReportDataQueryService>& report_data_query_service)
    -> TracerCoreRuntime {
  auto pipeline_api = std::make_shared<PipelineApi>(pipeline_workflow);
  auto query_api = std::make_shared<QueryApi>(repository, data_query);
  auto report_api =
      std::make_shared<ReportApi>(report_handler, report_data_query_service);
  auto tracer_exchange_api =
      std::make_shared<TracerExchangeApi>(tracer_exchange_service);
  return {std::move(pipeline_api), std::move(query_api), std::move(report_api),
          std::move(tracer_exchange_api)};
}

}  // namespace tracer_core::application::tests
