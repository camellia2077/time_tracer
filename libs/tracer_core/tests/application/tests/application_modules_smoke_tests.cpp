import tracer.core.application;

#include "application/dto/ingest_input_model.hpp"
#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/interfaces/i_report_handler.hpp"
#include "application/ports/i_converter_config_provider.hpp"
#include "application/ports/i_data_query_service.hpp"
#include "application/ports/i_database_health_checker.hpp"
#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "application/ports/i_time_sheet_repository.hpp"
#include "application/ports/i_validation_issue_reporter.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/repositories/i_project_repository.hpp"
#include "domain/types/app_options.hpp"
#include "domain/types/ingest_mode.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using tracer::core::application::modimporter::ImportService;
using tracer::core::application::modimporter::ImportStats;
using tracer::core::application::modimporter::ReplaceMonthTarget;
using tracer::core::application::modservice::ConverterService;

namespace app_pipeline = tracer::core::application::pipeline;
namespace app_tree = tracer::core::application::reporting::tree;
namespace app_use_cases = tracer::core::application::use_cases;
namespace app_workflow = tracer::core::application::workflow;
namespace app_ports = tracer_core::application::ports;

class SmokeWorkflowHandler final : public app_workflow::IWorkflowHandler {
 public:
  auto RunConverter(const std::string& /*input_path*/,
                    const AppOptions& /*options*/) -> void override {}

  auto RunDatabaseImport(const std::string& /*processed_path_str*/)
      -> void override {}

  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>& /*data_map*/)
      -> void override {}

  auto RunIngest(const std::string& /*source_path*/,
                 DateCheckMode /*date_check_mode*/,
                 bool /*save_processed*/,
                 IngestMode /*ingest_mode*/) -> void override {}

  auto RunValidateStructure(const std::string& /*source_path*/)
      -> void override {}

  auto RunValidateLogic(const std::string& /*source_path*/,
                        DateCheckMode /*date_check_mode*/) -> void override {}
};

class SmokeReportHandler final : public IReportHandler {
 public:
  auto RunDailyQuery(std::string_view /*date*/, ReportFormat /*format*/)
      -> std::string override {
    return "smoke-daily";
  }

  auto RunMonthlyQuery(std::string_view /*month*/, ReportFormat /*format*/)
      -> std::string override {
    return "smoke-monthly";
  }

  auto RunPeriodQuery(int /*days*/, ReportFormat /*format*/)
      -> std::string override {
    return "smoke-period";
  }

  auto RunWeeklyQuery(std::string_view /*iso_week*/, ReportFormat /*format*/)
      -> std::string override {
    return "smoke-weekly";
  }

  auto RunYearlyQuery(std::string_view /*year*/, ReportFormat /*format*/)
      -> std::string override {
    return "smoke-yearly";
  }

  auto RunPeriodQueries(const std::vector<int>& /*days_list*/,
                        ReportFormat /*format*/) -> std::string override {
    return "smoke-period-batch";
  }

  auto RunExportSingleDayReport(std::string_view /*date*/,
                                ReportFormat /*format*/) -> void override {}

  auto RunExportSingleMonthReport(std::string_view /*month*/,
                                  ReportFormat /*format*/) -> void override {}

  auto RunExportSinglePeriodReport(int /*days*/, ReportFormat /*format*/)
      -> void override {}

  auto RunExportSingleWeekReport(std::string_view /*iso_week*/,
                                 ReportFormat /*format*/) -> void override {}

  auto RunExportSingleYearReport(std::string_view /*year*/,
                                 ReportFormat /*format*/) -> void override {}

  auto RunExportAllDailyReportsQuery(ReportFormat /*format*/)
      -> void override {}

  auto RunExportAllMonthlyReportsQuery(ReportFormat /*format*/)
      -> void override {}

  auto RunExportAllPeriodReportsQuery(const std::vector<int>& /*days_list*/,
                                      ReportFormat /*format*/)
      -> void override {}

  auto RunExportAllWeeklyReportsQuery(ReportFormat /*format*/)
      -> void override {}

  auto RunExportAllYearlyReportsQuery(ReportFormat /*format*/)
      -> void override {}
};

class SmokeDataQueryService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  auto RunDataQuery(
      const tracer_core::core::dto::DataQueryRequest& /*request*/)
      -> tracer_core::core::dto::TextOutput override {
    return {.ok = true,
            .content = "smoke-data-query",
            .error_message = ""};
  }
};

class SmokeProjectRepository final : public IProjectRepository {
 public:
  auto GetAllProjects() -> std::vector<ProjectEntity> override {
    return {{.id = 1, .parent_id = std::nullopt, .name = "root"}};
  }
};

class SmokeProcessedDataLoader final : public app_ports::IProcessedDataLoader {
 public:
  auto LoadDailyLogs(const std::string& /*processed_path*/)
      -> app_ports::ProcessedDataLoadResult override {
    return {};
  }
};

class SmokeTimeSheetRepository final : public app_ports::ITimeSheetRepository {
 public:
  [[nodiscard]] auto IsDbOpen() const -> bool override { return true; }

  auto ImportData(const std::vector<DayData>& /*days*/,
                  const std::vector<TimeRecordInternal>& /*records*/)
      -> void override {}

  auto ReplaceMonthData(int /*year*/, int /*month*/,
                        const std::vector<DayData>& /*days*/,
                        const std::vector<TimeRecordInternal>& /*records*/)
      -> void override {}

  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(
      std::string_view /*date*/)
      const -> std::optional<app_ports::PreviousActivityTail> override {
    return std::nullopt;
  }
};

class SmokeDatabaseHealthChecker final
    : public app_ports::IDatabaseHealthChecker {
 public:
  auto CheckReady() -> app_ports::DatabaseHealthCheckResult override {
    return {.ok = true, .message = ""};
  }
};

class SmokeConverterConfigProvider final
    : public app_ports::IConverterConfigProvider {
 public:
  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override {
    return {};
  }
};

class SmokeIngestInputProvider final : public app_ports::IIngestInputProvider {
 public:
  [[nodiscard]] auto CollectTextInputs(
      const std::filesystem::path& /*input_root*/,
      std::string_view /*extension*/) const
      -> tracer_core::application::dto::IngestInputCollection override {
    return {};
  }
};

class SmokeProcessedDataStorage final
    : public app_ports::IProcessedDataStorage {
 public:
  auto WriteProcessedData(
      const std::map<std::string, std::vector<DailyLog>>& /*data*/,
      const std::filesystem::path& /*output_root*/)
      -> std::vector<std::filesystem::path> override {
    return {};
  }
};

class SmokeValidationIssueReporter final
    : public app_ports::IValidationIssueReporter {
 public:
  auto ReportStructureErrors(
      std::string_view /*display_label*/,
      const std::set<validator::Error>& /*errors*/) -> void override {}

  auto ReportLogicDiagnostics(
      std::string_view /*fallback_label*/,
      const std::vector<validator::Diagnostic>& /*diagnostics*/)
      -> void override {}
};

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestUseCasesAndServices(int& failures) {
  Expect(std::is_class_v<app_use_cases::ITracerCoreApi>,
         "ITracerCoreApi should be visible through module bridge.", failures);
  Expect(std::is_abstract_v<app_use_cases::ITracerCoreApi>,
         "ITracerCoreApi should remain an abstract interface.", failures);
  Expect(std::is_base_of_v<app_use_cases::ITracerCoreApi,
                           app_use_cases::TracerCoreApi>,
         "TracerCoreApi should keep ITracerCoreApi inheritance.", failures);
  Expect(std::is_class_v<ConverterService>,
         "ConverterService should be visible through module bridge.", failures);
  Expect(std::is_class_v<ImportService>,
         "ImportService should be visible through module bridge.", failures);

  auto (*build_error_message_fn)(std::string_view, std::string_view)
      -> std::string = &app_use_cases::helpers::BuildErrorMessage;
  auto (*build_text_failure_fn)(std::string_view)
      -> tracer_core::core::dto::TextOutput =
      &app_use_cases::helpers::BuildTextFailure;
  Expect(build_error_message_fn != nullptr,
         "BuildErrorMessage should be visible through the use_cases family.",
         failures);
  Expect(build_text_failure_fn != nullptr,
         "BuildTextFailure should be visible through the use_cases family.",
         failures);
  const auto helper_failure =
      app_use_cases::helpers::BuildTextFailure("RunDataQuery", "boom");
  Expect(!helper_failure.ok &&
             helper_failure.error_message == "RunDataQuery failed: boom",
         "BuildTextFailure should execute through the use_cases owning helper path.",
         failures);

  try {
    SmokeWorkflowHandler workflow_handler;
    SmokeReportHandler report_handler;
    auto project_repository = std::make_shared<SmokeProjectRepository>();
    auto data_query_service = std::make_shared<SmokeDataQueryService>();
    app_use_cases::TracerCoreApi core_api(workflow_handler, report_handler,
                                          project_repository,
                                          data_query_service);

    const auto data_query_result = core_api.RunDataQuery({});
    Expect(data_query_result.ok &&
               data_query_result.content == "smoke-data-query",
           "TracerCoreApi RunDataQuery should execute through the owning api path.",
           failures);

    const auto report_query_result = core_api.RunReportQuery(
        {.type = tracer_core::core::dto::ReportQueryType::kDay,
         .argument = "2026-03-10",
         .format = ReportFormat::kMarkdown});
    Expect(report_query_result.ok &&
               report_query_result.content == "smoke-daily",
           "TracerCoreApi RunReportQuery should execute through the owning api path.",
           failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr
        << "[FAIL] TracerCoreApi owning api path should construct and execute: "
        << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr
        << "[FAIL] TracerCoreApi owning api path should construct and execute: "
        << "unknown non-standard exception\n";
  }

  ImportStats stats;
  ReplaceMonthTarget target{.kYear = 2026, .kMonth = 3};
  Expect(stats.total_files == 0U && target.kMonth == 3,
         "Importer DTOs should be visible through module bridge.", failures);
}

void TestPipelineBridge(int& failures) {
  Expect(std::is_class_v<app_pipeline::PipelineRunSpec>,
         "PipelineRunSpec should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineSession>,
         "PipelineSession should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::PipelineOrchestrator>,
         "PipelineOrchestrator should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::InputCollectionStage>,
         "InputCollectionStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::StructureValidationStage>,
         "StructureValidationStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::ConversionStage>,
         "ConversionStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::CrossMonthLinkStage>,
         "CrossMonthLinkStage should be visible through the pipeline module.",
         failures);
  Expect(std::is_class_v<app_pipeline::LogicValidationStage>,
         "LogicValidationStage should be visible through the pipeline module.",
         failures);

  app_pipeline::PipelineSession context(
      std::filesystem::path("phase5-pipeline-module-output"));
  Expect(context.config.output_root ==
             std::filesystem::path("phase5-pipeline-module-output"),
         "PipelineSession constructor should preserve output_root.", failures);
  Expect(context.state.ingest_inputs.empty(),
         "PipelineSession state should initialize as empty.", failures);
  Expect(context.result.processed_data.empty(),
         "PipelineSession result should initialize as empty.", failures);
}

void TestReportingTreeBridge(int& failures) {
  Expect(std::is_class_v<app_tree::ProjectTreeNode>,
         "ProjectTreeNode should be visible through the reporting tree module.",
         failures);
  Expect(std::is_class_v<app_tree::ProjectTreeViewer>,
         "ProjectTreeViewer should be visible through the reporting tree module.",
         failures);

  const auto build_nodes_fn = &app_tree::BuildProjectTreeNodesFromReportTree;
  const auto find_nodes_fn = &app_tree::FindProjectTreeNodesByPath;
  const auto limit_depth_fn = &app_tree::LimitProjectTreeDepth;
  Expect(build_nodes_fn != nullptr,
         "BuildProjectTreeNodesFromReportTree should be exported.", failures);
  Expect(find_nodes_fn != nullptr,
         "FindProjectTreeNodesByPath should be exported.", failures);
  Expect(limit_depth_fn != nullptr,
         "LimitProjectTreeDepth should be exported.", failures);
}

void TestWorkflowBridge(int& failures) {
  Expect(std::is_class_v<app_workflow::IWorkflowHandler>,
         "IWorkflowHandler should be visible through the workflow family.",
         failures);
  Expect(std::is_abstract_v<app_workflow::IWorkflowHandler>,
         "IWorkflowHandler should remain an abstract interface.", failures);
  Expect(std::is_base_of_v<app_workflow::IWorkflowHandler,
                           app_workflow::WorkflowHandler>,
         "WorkflowHandler should keep IWorkflowHandler inheritance.", failures);
}

void TestWorkflowOwningPath(int& failures) {
  try {
    auto processed_data_loader = std::make_shared<SmokeProcessedDataLoader>();
    auto time_sheet_repository = std::make_shared<SmokeTimeSheetRepository>();
    auto database_health_checker =
        std::make_shared<SmokeDatabaseHealthChecker>();
    auto converter_config_provider =
        std::make_shared<SmokeConverterConfigProvider>();
    auto ingest_input_provider = std::make_shared<SmokeIngestInputProvider>();
    auto processed_data_storage =
        std::make_shared<SmokeProcessedDataStorage>();
    auto validation_issue_reporter =
        std::make_shared<SmokeValidationIssueReporter>();

    app_workflow::WorkflowHandler workflow_handler(
        std::filesystem::path("phase4-workflow-module-output"),
        std::move(processed_data_loader), std::move(time_sheet_repository),
        std::move(database_health_checker),
        std::move(converter_config_provider), std::move(ingest_input_provider),
        std::move(processed_data_storage),
        std::move(validation_issue_reporter));
    workflow_handler.RunDatabaseImport("phase4-workflow-module-smoke.json");
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr
        << "[FAIL] WorkflowHandler owning workflow path should construct and "
           "execute RunDatabaseImport: "
        << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr
        << "[FAIL] WorkflowHandler owning workflow path should construct and "
           "execute RunDatabaseImport: unknown non-standard exception\n";
  }
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestUseCasesAndServices(failures);
  TestPipelineBridge(failures);
  TestReportingTreeBridge(failures);
  TestWorkflowBridge(failures);
  TestWorkflowOwningPath(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_application_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
