import tracer.core.application;
import tracer.core.domain.model.daily_log;

#include "application/dto/ingest_input_model.hpp"
#include "application/ports/pipeline/i_converter_config_provider.hpp"
#include "application/ports/pipeline/i_database_health_checker.hpp"
#include "application/ports/pipeline/i_ingest_input_provider.hpp"
#include "application/ports/pipeline/i_processed_data_loader.hpp"
#include "application/ports/pipeline/i_processed_data_storage.hpp"
#include "application/ports/pipeline/i_time_sheet_repository.hpp"
#include "application/ports/pipeline/i_validation_issue_reporter.hpp"

#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using tracer::core::domain::modmodel::DailyLog;
namespace app_ports = tracer_core::application::ports;
namespace app_workflow = tracer::core::application::workflow;

class SmokeProcessedDataLoader final : public app_ports::IProcessedDataLoader {
 public:
  auto LoadDailyLogs(const std::string&)
      -> app_ports::ProcessedDataLoadResult override {
    return {};
  }
};

class SmokeTimeSheetRepository final : public app_ports::ITimeSheetRepository {
 public:
  [[nodiscard]] auto IsDbOpen() const -> bool override { return true; }
  auto ImportData(const std::vector<DayData>&,
                  const std::vector<TimeRecordInternal>&) -> void override {}
  auto ReplaceAllData(const std::vector<DayData>&,
                      const std::vector<TimeRecordInternal>&) -> void override {
  }
  auto ReplaceMonthData(int, int, const std::vector<DayData>&,
                        const std::vector<TimeRecordInternal>&)
      -> void override {}
  auto UpsertIngestSyncStatus(
      const tracer_core::core::dto::IngestSyncStatusEntry&) -> void override {}
  auto ReplaceIngestSyncStatuses(
      const std::vector<tracer_core::core::dto::IngestSyncStatusEntry>&)
      -> void override {}
  auto ClearIngestSyncStatus() -> void override {}
  [[nodiscard]] auto ListIngestSyncStatuses(
      const tracer_core::core::dto::IngestSyncStatusRequest&) const
      -> tracer_core::core::dto::IngestSyncStatusOutput override {
    return {.ok = true, .items = {}, .error_message = ""};
  }
  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(std::string_view) const
      -> std::optional<app_ports::PreviousActivityTail> override {
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
  auto InvalidateCache() -> void override {}
};

class SmokeIngestInputProvider final : public app_ports::IIngestInputProvider {
 public:
  [[nodiscard]] auto CollectTextInputs(const std::filesystem::path&,
                                       std::string_view) const
      -> tracer_core::application::dto::IngestInputCollection override {
    return {};
  }
};

class SmokeProcessedDataStorage final
    : public app_ports::IProcessedDataStorage {
 public:
  auto WriteProcessedData(const std::map<std::string, std::vector<DailyLog>>&,
                          const std::filesystem::path&)
      -> std::vector<std::filesystem::path> override {
    return {};
  }
};

class SmokeValidationIssueReporter final
    : public app_ports::IValidationIssueReporter {
 public:
  auto ReportStructureErrors(std::string_view,
                             const std::set<validator::Error>&)
      -> void override {}
  auto ReportLogicDiagnostics(std::string_view,
                              const std::vector<validator::Diagnostic>&)
      -> void override {}
};

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto RunWorkflowModuleSmoke(int& failures) -> void {
  Expect(std::is_class_v<app_workflow::IWorkflowHandler>,
         "IWorkflowHandler should remain visible.", failures);
  Expect(std::is_abstract_v<app_workflow::IWorkflowHandler>,
         "IWorkflowHandler should remain abstract.", failures);
  Expect(std::is_base_of_v<app_workflow::IWorkflowHandler,
                           app_workflow::WorkflowHandler>,
         "WorkflowHandler should preserve compatibility surface.", failures);

  try {
    auto processed_data_loader = std::make_shared<SmokeProcessedDataLoader>();
    auto time_sheet_repository = std::make_shared<SmokeTimeSheetRepository>();
    auto database_health_checker =
        std::make_shared<SmokeDatabaseHealthChecker>();
    auto converter_config_provider =
        std::make_shared<SmokeConverterConfigProvider>();
    auto ingest_input_provider = std::make_shared<SmokeIngestInputProvider>();
    auto processed_data_storage = std::make_shared<SmokeProcessedDataStorage>();
    auto validation_issue_reporter =
        std::make_shared<SmokeValidationIssueReporter>();

    app_workflow::WorkflowHandler pipeline_workflow(
        std::filesystem::path("phase4-workflow-module-output"),
        std::move(processed_data_loader), std::move(time_sheet_repository),
        std::move(database_health_checker),
        std::move(converter_config_provider), std::move(ingest_input_provider),
        std::move(processed_data_storage),
        std::move(validation_issue_reporter));
    pipeline_workflow.RunDatabaseImport("phase4-workflow-module-smoke.json");
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] WorkflowHandler should construct and execute "
                 "RunDatabaseImport: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] WorkflowHandler should construct and execute "
                 "RunDatabaseImport: "
              << "unknown non-standard exception\n";
  }
}

}  // namespace

auto main() -> int {
  int failures = 0;
  RunWorkflowModuleSmoke(failures);
  if (failures == 0) {
    std::cout << "[PASS] tracer_core_application_workflow_module_smoke_tests\n";
    return 0;
  }
  std::cerr
      << "[FAIL] tracer_core_application_workflow_module_smoke_tests failures: "
      << failures << '\n';
  return 1;
}
