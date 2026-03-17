// host/android_runtime_factory.cpp
import tracer.core.application.use_cases.api;
import tracer.core.application.workflow;
import tracer.adapters.io.runtime;
import tracer.core.infrastructure.config.file_converter_config_provider;
import tracer.core.infrastructure.logging;
import tracer.core.infrastructure.persistence.runtime;
import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.reports.data_querying;
import tracer.core.infrastructure.reports.dto;
import tracer.core.infrastructure.reports.exporting;
import tracer.core.infrastructure.reports.querying;
import tracer.core.infrastructure.platform.android.clock;

#include "host/android_runtime_factory.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "host/android_runtime_factory_internal.hpp"
#include "application/interfaces/i_report_handler.hpp"
#include "application/ports/i_report_formatter_registry.hpp"
#include "application/ports/logger.hpp"
#include "application/reporting/report_handler.hpp"
#include "domain/ports/diagnostics.hpp"
#include "infrastructure/persistence/sqlite_data_query_service.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"

namespace {

namespace fs = std::filesystem;
namespace app_use_cases = tracer::core::application::use_cases;
namespace app_workflow = tracer::core::application::workflow;
namespace adapters_runtime = tracer::adapters::io::modruntime;
namespace infra_persistence_runtime = tracer::core::infrastructure::persistence;
namespace infra_persistence_write = tracer::core::infrastructure::persistence;
namespace infra_reports = tracer::core::infrastructure::reports;
namespace infra_platform = tracer::core::infrastructure::modplatform;
using FileConverterConfigProvider =
    tracer::core::infrastructure::config::FileConverterConfigProvider;
namespace infra_logging = tracer::core::infrastructure::logging;

auto ResolveRuntimeDataRoot(const fs::path& db_path,
                            const fs::path& fallback_output_root) -> fs::path {
  const fs::path kDbDir = db_path.parent_path();
  if (kDbDir.empty()) {
    return fallback_output_root;
  }

  const fs::path kRuntimeRoot = kDbDir.parent_path();
  if (kRuntimeRoot.empty()) {
    return fallback_output_root;
  }
  return kRuntimeRoot;
}

auto BuildIsoUtcTimestampForFilename() -> std::string {
  const auto kNow = std::chrono::system_clock::now();
  const std::time_t kNowTime = std::chrono::system_clock::to_time_t(kNow);

  std::tm utc_time{};
#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&utc_time, &kNowTime);
#else
  gmtime_r(&kNowTime, &utc_time);
#endif

  std::ostringstream stream;
  stream << std::put_time(&utc_time, "%Y-%m-%dT%H-%M-%SZ");
  return stream.str();
}

auto BuildRunScopedErrorLogPath(const fs::path& logs_root) -> fs::path {
  return logs_root / ("errors-" + BuildIsoUtcTimestampForFilename() + ".log");
}

struct AndroidRuntimeState {
  std::shared_ptr<app_workflow::IWorkflowHandler> workflow_handler;
  std::shared_ptr<IReportHandler> report_handler;
  std::shared_ptr<ReportCatalog> report_catalog;
};

}  // namespace

namespace infrastructure::bootstrap {

auto BuildAndroidRuntime(const AndroidRuntimeRequest& request)
    -> AndroidRuntime {
  // Runtime bootstrap must stay side-effect free with respect to ingest
  // persistence. Creating the runtime is not permission to create the ingest
  // database; database creation belongs only to the post-validation write phase.
  const fs::path kOutputRoot =
      android_runtime_detail::ResolveOutputRoot(request.output_root);
  const fs::path kDbPath =
      android_runtime_detail::ResolveDbPath(request.db_path, kOutputRoot);
  const fs::path kRuntimeDataRoot =
      ResolveRuntimeDataRoot(kDbPath, kOutputRoot);
  const fs::path kErrorLogsRoot = kRuntimeDataRoot / "logs";
  const android_runtime_detail::AndroidRuntimeConfigPaths kRuntimeConfigPaths =
      android_runtime_detail::ResolveAndroidRuntimeConfigPaths(
          request.converter_config_toml_path);
  const fs::path kConverterConfigTomlPath =
      kRuntimeConfigPaths.converter_config_toml_path;

  tracer_core::application::ports::SetLogger(request.logger);
  tracer_core::domain::ports::SetDiagnosticsSink(request.diagnostics_sink);
  tracer_core::domain::ports::SetErrorReportWriter(
      request.error_report_writer
          ? request.error_report_writer
          : std::make_shared<infra_logging::FileErrorReportWriter>(
                BuildRunScopedErrorLogPath(kErrorLogsRoot),
                kErrorLogsRoot / "errors-latest.log"));
  // Reset session-level diagnostics state for this run.
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  tracer_core::domain::ports::ClearDiagnosticsDedup();

  auto processed_data_loader = adapters_runtime::CreateProcessedDataLoader();
  auto time_sheet_repository =
      std::make_shared<infra_persistence_write::SqliteTimeSheetRepository>(
          kDbPath.string());
  auto database_health_checker = std::make_shared<
      infra_persistence_runtime::SqliteDatabaseHealthChecker>(
      kDbPath.string());
  auto converter_config_provider =
      std::make_shared<FileConverterConfigProvider>(
          kConverterConfigTomlPath, std::unordered_map<fs::path, fs::path>{});
  // Fail fast during runtime bootstrap if converter TOML is invalid.
  static_cast<void>(converter_config_provider->LoadConverterConfig());
  auto ingest_input_provider = adapters_runtime::CreateTxtIngestInputProvider();
  auto processed_data_storage = adapters_runtime::CreateProcessedDataStorage();
  auto validation_issue_reporter =
      std::make_shared<infra_logging::ValidationIssueReporter>();

  auto workflow = std::make_shared<app_workflow::WorkflowHandler>(
      kOutputRoot, std::move(processed_data_loader),
      std::move(time_sheet_repository), std::move(database_health_checker),
      std::move(converter_config_provider), std::move(ingest_input_provider),
      std::move(processed_data_storage), std::move(validation_issue_reporter));

  auto platform_clock =
      std::make_shared<infra_platform::AndroidPlatformClock>();

  auto report_catalog = std::make_shared<ReportCatalog>(
      android_runtime_detail::BuildAndroidReportCatalog(kOutputRoot,
                                                        kRuntimeConfigPaths));

  auto report_query_service =
      std::make_unique<infra_reports::LazySqliteReportQueryService>(
          kDbPath, report_catalog, platform_clock);
  auto exporter = std::make_unique<infra_reports::Exporter>(kOutputRoot);
  auto report = std::make_shared<ReportHandler>(std::move(report_query_service),
                                                std::move(exporter));

  auto project_repository =
      std::make_shared<infra_persistence_runtime::SqliteProjectRepository>(
          kDbPath.string());
  auto data_query_service =
      std::make_shared<infrastructure::persistence::SqliteDataQueryService>(
          kDbPath, kConverterConfigTomlPath);
  auto report_data_query_service =
      std::make_shared<infra_reports::LazySqliteReportDataQueryService>(
          kDbPath, platform_clock);
  auto static_formatter_registrar = std::make_shared<
      infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
      kRuntimeConfigPaths.formatter_policy);
  auto formatter_registry =
      tracer_core::application::ports::CreateReportFormatterRegistry(
          static_formatter_registrar);
  formatter_registry->RegisterFormatters();
  auto report_dto_formatter =
      infra_reports::CreateReportDtoFormatter(*report_catalog);
  auto report_exporter_for_dto =
      std::make_shared<infra_reports::Exporter>(kOutputRoot);
  auto report_export_writer = std::make_shared<infra_reports::ReportDtoExportWriter>(
      report_dto_formatter, report_exporter_for_dto);

  AndroidRuntime runtime;
  runtime.core_api = std::make_shared<app_use_cases::TracerCoreApi>(
      *workflow, *report, project_repository, std::move(data_query_service),
      std::move(report_data_query_service), std::move(report_dto_formatter),
      std::move(report_export_writer));

  auto runtime_state = std::make_shared<AndroidRuntimeState>();
  runtime_state->workflow_handler = std::move(workflow);
  runtime_state->report_handler = std::move(report);
  runtime_state->report_catalog = std::move(report_catalog);
  runtime.runtime_state = std::move(runtime_state);
  return runtime;
}

}  // namespace infrastructure::bootstrap
