// host/android_runtime_factory.cpp
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
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_report_formatter_registry.hpp"
#include "application/ports/logger.hpp"
#include "application/reporting/report_handler.hpp"
#include "application/use_cases/tracer_core_api.hpp"
#include "application/workflow_handler.hpp"
#include "domain/ports/diagnostics.hpp"
#include "infrastructure/config/file_converter_config_provider.hpp"
#include "infrastructure/io/processed_data_io.hpp"
#include "infrastructure/io/txt_ingest_input_provider.hpp"
#include "infrastructure/logging/file_error_report_writer.hpp"
#include "infrastructure/logging/validation_issue_reporter.hpp"
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"
#include "infrastructure/persistence/sqlite_data_query_service.hpp"
#include "infrastructure/persistence/sqlite_database_health_checker.hpp"
#include "infrastructure/persistence/sqlite_time_sheet_repository.hpp"
#include "infrastructure/platform/android/android_platform_clock.hpp"
#include "infrastructure/reports/exporter.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"
#include "infrastructure/reports/lazy_sqlite_report_data_query_service.hpp"
#include "infrastructure/reports/lazy_sqlite_report_query_service.hpp"
#include "infrastructure/reports/report_dto_export_writer.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"

namespace {

namespace fs = std::filesystem;

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
  std::shared_ptr<IWorkflowHandler> workflow_handler;
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
          : std::make_shared<infrastructure::logging::FileErrorReportWriter>(
                BuildRunScopedErrorLogPath(kErrorLogsRoot),
                kErrorLogsRoot / "errors-latest.log"));
  // Reset session-level diagnostics state for this run.
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  tracer_core::domain::ports::ClearDiagnosticsDedup();

  auto processed_data_loader =
      std::make_shared<infrastructure::io::ProcessedDataLoader>();
  auto time_sheet_repository =
      std::make_shared<infrastructure::persistence::SqliteTimeSheetRepository>(
          kDbPath.string());
  auto database_health_checker = std::make_shared<
      infrastructure::persistence::SqliteDatabaseHealthChecker>(
      kDbPath.string());
  auto converter_config_provider =
      std::make_shared<infrastructure::config::FileConverterConfigProvider>(
          kConverterConfigTomlPath, std::unordered_map<fs::path, fs::path>{});
  // Fail fast during runtime bootstrap if converter TOML is invalid.
  static_cast<void>(converter_config_provider->LoadConverterConfig());
  auto ingest_input_provider =
      std::make_shared<infrastructure::io::TxtIngestInputProvider>();
  auto processed_data_storage =
      std::make_shared<infrastructure::io::ProcessedDataStorage>();
  auto validation_issue_reporter =
      std::make_shared<infrastructure::logging::ValidationIssueReporter>();

  auto workflow = std::make_shared<WorkflowHandler>(
      kOutputRoot, std::move(processed_data_loader),
      std::move(time_sheet_repository), std::move(database_health_checker),
      std::move(converter_config_provider), std::move(ingest_input_provider),
      std::move(processed_data_storage), std::move(validation_issue_reporter));

  auto platform_clock =
      std::make_shared<infrastructure::platform::AndroidPlatformClock>();

  auto report_catalog = std::make_shared<ReportCatalog>(
      android_runtime_detail::BuildAndroidReportCatalog(kOutputRoot,
                                                        kRuntimeConfigPaths));

  auto report_query_service =
      std::make_unique<infrastructure::reports::LazySqliteReportQueryService>(
          kDbPath, report_catalog, platform_clock);
  auto exporter = std::make_unique<Exporter>(kOutputRoot);
  auto report = std::make_shared<ReportHandler>(std::move(report_query_service),
                                                std::move(exporter));

  auto project_repository =
      std::make_shared<SqliteProjectRepository>(kDbPath.string());
  auto data_query_service =
      std::make_shared<infrastructure::persistence::SqliteDataQueryService>(
          kDbPath, kConverterConfigTomlPath);
  auto report_data_query_service =
      std::make_shared<
          infrastructure::reports::LazySqliteReportDataQueryService>(
          kDbPath, platform_clock);
  auto static_formatter_registrar = std::make_shared<
      infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
      kRuntimeConfigPaths.formatter_policy);
  auto formatter_registry =
      tracer_core::application::ports::CreateReportFormatterRegistry(
          static_formatter_registrar);
  formatter_registry->RegisterFormatters();
  auto report_dto_formatter =
      std::make_shared<infrastructure::reports::ReportDtoFormatter>(
          *report_catalog);
  auto report_exporter_for_dto = std::make_shared<Exporter>(kOutputRoot);
  auto report_export_writer =
      std::make_shared<infrastructure::reports::ReportDtoExportWriter>(
          report_dto_formatter, report_exporter_for_dto);

  AndroidRuntime runtime;
  runtime.core_api = std::make_shared<TracerCoreApi>(
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
