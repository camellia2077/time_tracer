// application/tests/support/fakes.cpp
#include "application/tests/support/fakes.hpp"

#include <stdexcept>

namespace time_tracer::application::tests {

auto FakeWorkflowHandler::RunConverter(const std::string& input_path,
                                       const AppOptions& options) -> void {
  ++convert_call_count;
  last_converter_input = input_path;
  last_converter_options = options;
  if (fail_convert) {
    throw std::runtime_error("convert failed");
  }
}

auto FakeWorkflowHandler::RunDatabaseImport(
    const std::string& processed_path_str) -> void {
  ++import_call_count;
  last_import_path = processed_path_str;
  if (fail_import) {
    throw std::runtime_error("import failed");
  }
}

auto FakeWorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& /*data_map*/) -> void {}

auto FakeWorkflowHandler::RunIngest(const std::string& source_path,
                                    DateCheckMode date_check_mode,
                                    bool save_processed, IngestMode ingest_mode)
    -> void {
  ++ingest_call_count;
  last_ingest_input = source_path;
  last_ingest_mode = date_check_mode;
  last_ingest_save_processed = save_processed;
  last_ingest_import_mode = ingest_mode;
  if (fail_ingest) {
    throw std::runtime_error("ingest failed");
  }
}

auto FakeWorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  ++validate_structure_call_count;
  last_validate_structure_input = source_path;
  if (fail_validate_structure) {
    throw std::runtime_error("validate structure failed");
  }
}

auto FakeWorkflowHandler::RunValidateLogic(const std::string& source_path,
                                           DateCheckMode date_check_mode)
    -> void {
  ++validate_logic_call_count;
  last_validate_logic_input = source_path;
  last_validate_logic_mode = date_check_mode;
  if (fail_validate_logic) {
    throw std::runtime_error("validate logic failed");
  }
}

auto FakeReportHandler::RunDailyQuery(std::string_view /*date*/,
                                      ReportFormat /*format*/) -> std::string {
  if (fail_query) {
    throw std::runtime_error("daily query failed");
  }
  return daily_query_result;
}

auto FakeReportHandler::RunMonthlyQuery(std::string_view /*month*/,
                                        ReportFormat /*format*/)
    -> std::string {
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
  if (fail_query) {
    throw std::runtime_error("weekly query failed");
  }
  return weekly_query_result;
}

auto FakeReportHandler::RunYearlyQuery(std::string_view /*year*/,
                                       ReportFormat /*format*/) -> std::string {
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

auto FakeReportHandler::RunExportSingleDayReport(std::string_view /*date*/,
                                                 ReportFormat /*format*/)
    -> void {
  ++daily_export_count;
  if (fail_export) {
    throw std::runtime_error("day export failed");
  }
}

auto FakeReportHandler::RunExportSingleMonthReport(std::string_view /*month*/,
                                                   ReportFormat /*format*/)
    -> void {
  ++monthly_export_count;
  if (fail_export) {
    throw std::runtime_error("month export failed");
  }
}

auto FakeReportHandler::RunExportSinglePeriodReport(int /*days*/,
                                                    ReportFormat /*format*/)
    -> void {
  ++recent_export_count;
  if (fail_export) {
    throw std::runtime_error("recent export failed");
  }
}

auto FakeReportHandler::RunExportSingleWeekReport(std::string_view /*iso_week*/,
                                                  ReportFormat /*format*/)
    -> void {
  ++weekly_export_count;
  if (fail_export) {
    throw std::runtime_error("week export failed");
  }
}

auto FakeReportHandler::RunExportSingleYearReport(std::string_view /*year*/,
                                                  ReportFormat /*format*/)
    -> void {
  ++yearly_export_count;
  if (fail_export) {
    throw std::runtime_error("year export failed");
  }
}

auto FakeReportHandler::RunExportAllDailyReportsQuery(ReportFormat /*format*/)
    -> void {
  if (fail_export) {
    throw std::runtime_error("all-day export failed");
  }
}

auto FakeReportHandler::RunExportAllMonthlyReportsQuery(ReportFormat /*format*/)
    -> void {
  if (fail_export) {
    throw std::runtime_error("all-month export failed");
  }
}

auto FakeReportHandler::RunExportAllPeriodReportsQuery(
    const std::vector<int>& /*days_list*/, ReportFormat /*format*/) -> void {
  if (fail_export) {
    throw std::runtime_error("all-recent export failed");
  }
}

auto FakeReportHandler::RunExportAllWeeklyReportsQuery(ReportFormat /*format*/)
    -> void {
  if (fail_export) {
    throw std::runtime_error("all-week export failed");
  }
}

auto FakeReportHandler::RunExportAllYearlyReportsQuery(ReportFormat /*format*/)
    -> void {
  if (fail_export) {
    throw std::runtime_error("all-year export failed");
  }
}

auto FakeDataQueryService::RunDataQuery(
    const time_tracer::core::dto::DataQueryRequest& request)
    -> time_tracer::core::dto::TextOutput {
  ++call_count;
  last_request = request;
  if (fail_query) {
    throw std::runtime_error("data query failed");
  }
  return response;
}

auto FakeProjectRepository::GetAllProjects() -> std::vector<ProjectEntity> {
  return {{.id = 1, .parent_id = std::nullopt, .name = "root"}};
}

auto BuildCoreApi(FakeWorkflowHandler& workflow_handler,
                  FakeReportHandler& report_handler,
                  const std::shared_ptr<FakeProjectRepository>& repository,
                  const std::shared_ptr<FakeDataQueryService>& data_query)
    -> TimeTracerCoreApi {
  return {workflow_handler, report_handler, repository, data_query};
}

}  // namespace time_tracer::application::tests
