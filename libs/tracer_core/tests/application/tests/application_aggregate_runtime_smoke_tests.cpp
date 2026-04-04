import tracer.core.application;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.repositories.project_repository;
import tracer.core.domain.types.app_options;
import tracer.core.domain.types.date_check_mode;
import tracer.core.domain.types.ingest_mode;

#include "application/aggregate_runtime/tracer_core_runtime.hpp"
#include "application/compat/reporting/i_report_handler.hpp"
#include "application/dto/query_requests.hpp"
#include "application/dto/reporting_requests.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "application/pipeline/i_pipeline_workflow.hpp"
#include "application/ports/query/i_data_query_service.hpp"

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using tracer::core::domain::modmodel::DailyLog;
using tracer::core::domain::modrepos::IProjectRepository;
using tracer::core::domain::modrepos::ProjectEntity;
using tracer::core::domain::modtypes::AppOptions;
using tracer::core::domain::modtypes::DateCheckMode;
using tracer::core::domain::modtypes::IngestMode;

namespace app_use_cases = tracer::core::application::use_cases;

class SmokePipelineWorkflow final
    : public tracer::core::application::workflow::IWorkflowHandler {
 public:
  auto RunConverter(const std::string&, const AppOptions&) -> void override {}
  auto RunDatabaseImport(const std::string&) -> void override {}
  auto RunDatabaseImportFromMemory(
      const std::map<std::string, std::vector<DailyLog>>&) -> void override {}
  auto RunIngest(const std::string&, DateCheckMode, bool, IngestMode)
      -> void override {}
  auto RunIngestSyncStatusQuery(
      const tracer_core::core::dto::IngestSyncStatusRequest&)
      -> tracer_core::core::dto::IngestSyncStatusOutput override {
    return {.ok = true, .items = {}, .error_message = ""};
  }
  auto ClearIngestSyncStatus() -> void override {}
  auto RunIngestReplacingAll(const std::string&, DateCheckMode, bool)
      -> void override {}
  auto RunValidateStructure(const std::string&) -> void override {}
  auto RunValidateLogic(const std::string&, DateCheckMode) -> void override {}
  auto RunRecordActivityAtomically(
      const tracer_core::core::dto::RecordActivityAtomicallyRequest&)
      -> tracer_core::core::dto::RecordActivityAtomicallyResponse override {
    return {.ok = true,
            .message = "record: ok\nsync: ok",
            .operation_id = "smoke-txn",
            .warnings = {},
            .rollback_failed = false,
            .retained_transaction_root = std::nullopt};
  }
  auto InstallActiveConverterConfig(
      const tracer::core::application::pipeline::
          ActiveConverterConfigInstallRequest&) -> void override {}
};

class SmokeReportHandler final : public IReportHandler {
 public:
  auto RunDailyQuery(std::string_view, ReportFormat) -> std::string override {
    return "smoke-daily";
  }
  auto RunMonthlyQuery(std::string_view, ReportFormat) -> std::string override {
    return "smoke-monthly";
  }
  auto RunPeriodQuery(int, ReportFormat) -> std::string override {
    return "smoke-period";
  }
  auto RunWeeklyQuery(std::string_view, ReportFormat) -> std::string override {
    return "smoke-weekly";
  }
  auto RunYearlyQuery(std::string_view, ReportFormat) -> std::string override {
    return "smoke-yearly";
  }
  auto RunPeriodQueries(const std::vector<int>&, ReportFormat)
      -> std::string override {
    return "smoke-period-batch";
  }
};

class SmokeDataQueryService final
    : public tracer_core::application::ports::IDataQueryService {
 public:
  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest&)
      -> tracer_core::core::dto::TextOutput override {
    return {.ok = true, .content = "smoke-data-query", .error_message = ""};
  }
};

class SmokeProjectRepository final : public IProjectRepository {
 public:
  auto GetAllProjects() -> std::vector<ProjectEntity> override {
    return {{.id = 1, .parent_id = std::nullopt, .name = "root"}};
  }
};

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto RunAggregateRuntimeSmoke(int& failures) -> void {
  Expect(std::is_class_v<app_use_cases::ITracerCoreRuntime>,
         "ITracerCoreRuntime should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_abstract_v<app_use_cases::ITracerCoreRuntime>,
         "ITracerCoreRuntime should remain abstract.", failures);
  Expect(std::is_base_of_v<app_use_cases::ITracerCoreRuntime,
                           app_use_cases::TracerCoreRuntime>,
         "TracerCoreRuntime should inherit ITracerCoreRuntime.", failures);
  Expect(std::is_class_v<app_use_cases::IPipelineApi>,
         "IPipelineApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::IQueryApi>,
         "IQueryApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::IReportApi>,
         "IReportApi should be visible through aggregate runtime path.",
         failures);
  Expect(std::is_class_v<app_use_cases::ITracerExchangeApi>,
         "ITracerExchangeApi should be visible through aggregate runtime path.",
         failures);

  auto (*build_error_message_fn)(std::string_view, std::string_view)
      ->std::string = &app_use_cases::helpers::BuildErrorMessage;
  auto (*build_text_failure_fn)(std::string_view)
      ->tracer_core::core::dto::TextOutput =
      &app_use_cases::helpers::BuildTextFailure;
  Expect(build_error_message_fn != nullptr,
         "BuildErrorMessage should remain exported.", failures);
  Expect(build_text_failure_fn != nullptr,
         "BuildTextFailure should remain exported.", failures);

  const auto helper_failure =
      app_use_cases::helpers::BuildTextFailure("RunDataQuery", "boom");
  Expect(!helper_failure.ok &&
             helper_failure.error_message == "RunDataQuery failed: boom",
         "BuildTextFailure should preserve aggregate failure wording.",
         failures);

  try {
    SmokePipelineWorkflow pipeline_workflow;
    SmokeReportHandler report_handler;
    auto project_repository = std::make_shared<SmokeProjectRepository>();
    auto data_query_service = std::make_shared<SmokeDataQueryService>();
    auto pipeline_api =
        std::make_shared<app_use_cases::PipelineApi>(pipeline_workflow);
    auto query_api = std::make_shared<app_use_cases::QueryApi>(
        project_repository, data_query_service);
    auto report_api =
        std::make_shared<app_use_cases::ReportApi>(report_handler);
    auto tracer_exchange_api =
        std::make_shared<app_use_cases::TracerExchangeApi>();
    app_use_cases::TracerCoreRuntime runtime_api(
        std::move(pipeline_api), std::move(query_api), std::move(report_api),
        std::move(tracer_exchange_api));

    const auto data_query_result = runtime_api.query().RunDataQuery({});
    Expect(
        data_query_result.ok && data_query_result.content == "smoke-data-query",
        "Aggregate runtime should dispatch query API.", failures);

    const auto report_query_result = runtime_api.report().RunReportQuery(
        {.type = tracer_core::core::dto::ReportQueryType::kDay,
         .argument = "2026-03-10",
         .format = ReportFormat::kMarkdown});
    Expect(
        report_query_result.ok && report_query_result.content == "smoke-daily",
        "Aggregate runtime should dispatch report API.", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Aggregate runtime smoke should construct and execute: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Aggregate runtime smoke should construct and execute: "
              << "unknown non-standard exception\n";
  }
}

}  // namespace

auto main() -> int {
  int failures = 0;
  RunAggregateRuntimeSmoke(failures);
  if (failures == 0) {
    std::cout
        << "[PASS] tracer_core_application_aggregate_runtime_smoke_tests\n";
    return 0;
  }
  std::cerr << "[FAIL] tracer_core_application_aggregate_runtime_smoke_tests "
               "failures: "
            << failures << '\n';
  return 1;
}
