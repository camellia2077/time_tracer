// application/tests/support/fakes.hpp
#ifndef APPLICATION_TESTS_SUPPORT_FAKES_H_
#define APPLICATION_TESTS_SUPPORT_FAKES_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/interfaces/i_report_handler.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_data_query_service.hpp"
#include "application/use_cases/tracer_core_api.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/repositories/i_project_repository.hpp"
#include "domain/types/ingest_mode.hpp"

namespace tracer_core::application::tests {

class FakeWorkflowHandler final : public IWorkflowHandler {
 public:
  bool fail_convert = false;
  bool fail_ingest = false;
  bool fail_import = false;
  bool fail_validate_structure = false;
  bool fail_validate_logic = false;

  std::string last_converter_input;
  AppOptions last_converter_options;
  std::string last_ingest_input;
  DateCheckMode last_ingest_mode = DateCheckMode::kNone;
  bool last_ingest_save_processed = false;
  IngestMode last_ingest_import_mode = IngestMode::kStandard;
  std::string last_import_path;
  std::string last_validate_structure_input;
  std::string last_validate_logic_input;
  DateCheckMode last_validate_logic_mode = DateCheckMode::kNone;

  int convert_call_count = 0;
  int ingest_call_count = 0;
  int import_call_count = 0;
  int validate_structure_call_count = 0;
  int validate_logic_call_count = 0;

  auto RunConverter(const std::string& input_path, const AppOptions& options)
      -> void override;
  auto RunDatabaseImport(const std::string& processed_path_str)
      -> void override;
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& data_map)
      -> void override;
  auto RunIngest(const std::string& source_path, DateCheckMode date_check_mode,
                 bool save_processed, IngestMode ingest_mode) -> void override;
  auto RunValidateStructure(const std::string& source_path) -> void override;
  auto RunValidateLogic(const std::string& source_path,
                        DateCheckMode date_check_mode) -> void override;
};

class FakeReportHandler final : public IReportHandler {
 public:
  bool fail_query = false;
  bool fail_period_batch_query = false;
  bool fail_export = false;

  std::string daily_query_result = "daily";
  std::string monthly_query_result = "monthly";
  std::string weekly_query_result = "weekly";
  std::string yearly_query_result = "yearly";
  std::string recent_query_result = "recent";
  std::string period_batch_result = "period-batch";

  int daily_export_count = 0;
  int monthly_export_count = 0;
  int weekly_export_count = 0;
  int yearly_export_count = 0;
  int recent_export_count = 0;

  auto RunDailyQuery(std::string_view date, ReportFormat format)
      -> std::string override;
  auto RunMonthlyQuery(std::string_view month, ReportFormat format)
      -> std::string override;
  auto RunPeriodQuery(int days, ReportFormat format) -> std::string override;
  auto RunWeeklyQuery(std::string_view iso_week, ReportFormat format)
      -> std::string override;
  auto RunYearlyQuery(std::string_view year, ReportFormat format)
      -> std::string override;
  auto RunPeriodQueries(const std::vector<int>& days_list, ReportFormat format)
      -> std::string override;
  auto RunExportSingleDayReport(std::string_view date, ReportFormat format)
      -> void override;
  auto RunExportSingleMonthReport(std::string_view month, ReportFormat format)
      -> void override;
  auto RunExportSinglePeriodReport(int days, ReportFormat format)
      -> void override;
  auto RunExportSingleWeekReport(std::string_view iso_week, ReportFormat format)
      -> void override;
  auto RunExportSingleYearReport(std::string_view year, ReportFormat format)
      -> void override;
  auto RunExportAllDailyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllMonthlyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllPeriodReportsQuery(const std::vector<int>& days_list,
                                      ReportFormat format) -> void override;
  auto RunExportAllWeeklyReportsQuery(ReportFormat format) -> void override;
  auto RunExportAllYearlyReportsQuery(ReportFormat format) -> void override;
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

auto BuildCoreApi(FakeWorkflowHandler& workflow_handler,
                  FakeReportHandler& report_handler,
                  const std::shared_ptr<FakeProjectRepository>& repository,
                  const std::shared_ptr<FakeDataQueryService>& data_query)
    -> TracerCoreApi;

}  // namespace tracer_core::application::tests

#endif  // APPLICATION_TESTS_SUPPORT_FAKES_H_
