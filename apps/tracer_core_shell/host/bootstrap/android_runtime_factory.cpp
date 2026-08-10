// host/bootstrap/android_runtime_factory.cpp
import tracer.core.application.use_cases.api;
import tracer.core.application.workflow;
import tracer.adapters.io.runtime;
import tracer.core.infrastructure.config.file_converter_config_provider;
import tracer.core.infrastructure.logging;
import tracer.core.infrastructure.persistence.runtime;
import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.insights.data_querying;
import tracer.core.infrastructure.insights.dto;
import tracer.core.infrastructure.insights.querying;
import tracer.core.infrastructure.platform.android.clock;

#include "host/bootstrap/android_runtime_factory.hpp"

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "host/bootstrap/android_runtime_factory_internal.hpp"
#include "application/compat/insights/i_insights_handler.hpp"
#include "application/ports/insights/i_insights_formatter_registry.hpp"
#include "application/runtime_bridge/logger.hpp"
#include "application/insights/insights_handler.hpp"
#include "domain/ports/diagnostics.hpp"
#include "infra/exchange/tracer_exchange_service.hpp"
#include "infra/query/data/repository/query_runtime_service.hpp"
#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"

namespace {

namespace fs = std::filesystem;
namespace app_use_cases = tracer::core::application::use_cases;
namespace app_workflow = tracer::core::application::workflow;
namespace adapters_runtime = tracer::adapters::io::modruntime;
namespace infra_persistence_runtime = tracer::core::infrastructure::persistence;
namespace infra_persistence_write = tracer::core::infrastructure::persistence;
namespace infra_insights = tracer::core::infrastructure::insights;
namespace infra_platform = tracer::core::infrastructure::modplatform;
namespace infra_logging = tracer::core::infrastructure::logging;
using FileConverterConfigProvider =
    tracer::core::infrastructure::config::FileConverterConfigProvider;

struct AndroidRuntimeState {
  std::shared_ptr<app_workflow::IWorkflowHandler> workflow_handler;
  std::shared_ptr<IInsightsHandler> insights_handler;
  std::shared_ptr<InsightsCatalog> insights_catalog;
};

struct AndroidPipelineRuntimeState {
  std::shared_ptr<app_workflow::IWorkflowHandler> workflow_handler;
};

auto BuildAndroidPipelineState(const fs::path& output_root,
                               const fs::path& db_path,
                               const fs::path& converter_config_path)
    -> std::shared_ptr<AndroidPipelineRuntimeState> {
  auto processed_data_loader = adapters_runtime::CreateProcessedDataLoader();
  auto time_sheet_repository =
      std::make_shared<infra_persistence_write::SqliteTimeSheetRepository>(
          db_path.string());
  auto ingest_runtime_repository =
      std::make_shared<infra_persistence_runtime::SqliteIngestRuntimeRepository>(
          db_path.string());
  auto database_health_checker =
      std::make_shared<infra_persistence_runtime::SqliteDatabaseHealthChecker>(
          db_path.string());
  auto converter_config_provider =
      std::make_shared<FileConverterConfigProvider>(
          converter_config_path, std::unordered_map<fs::path, fs::path>{});
  // Fail fast during pipeline bootstrap if converter TOML or the user
  // activity hierarchy is invalid.
  static_cast<void>(converter_config_provider->LoadConverterConfig());
  auto ingest_input_provider = adapters_runtime::CreateTxtIngestInputProvider();
  auto processed_data_storage = adapters_runtime::CreateProcessedDataStorage();
  auto validation_issue_reporter =
      std::make_shared<infra_logging::ValidationIssueReporter>();

  auto state = std::make_shared<AndroidPipelineRuntimeState>();
  state->workflow_handler = std::make_shared<app_workflow::WorkflowHandler>(
      output_root, std::move(processed_data_loader),
      std::move(time_sheet_repository), std::move(ingest_runtime_repository),
      std::move(database_health_checker),
      std::move(converter_config_provider), std::move(ingest_input_provider),
      std::move(processed_data_storage), std::move(validation_issue_reporter));
  return state;
}

}  // namespace

namespace infrastructure::bootstrap {

auto BuildAndroidRuntime(const AndroidRuntimeRequest& request)
    -> AndroidRuntime {
  // Runtime bootstrap must stay side-effect free with respect to ingest
  // persistence. Creating the runtime is not permission to create the ingest
  // database; database creation belongs only to the post-validation write
  // phase.
  const fs::path kOutputRoot =
      android_runtime_detail::ResolveOutputRoot(request.output_root);
  const fs::path kDbPath =
      android_runtime_detail::ResolveDbPath(request.db_path, kOutputRoot);
  const android_runtime_detail::AndroidRuntimeConfigPaths kRuntimeConfigPaths =
      android_runtime_detail::ResolveAndroidRuntimeConfigPaths(
          request.converter_config_toml_path);
  const fs::path kConverterConfigTomlPath =
      kRuntimeConfigPaths.converter_config_toml_path;

  tracer_core::application::runtime_bridge::SetLogger(request.logger);
  tracer_core::domain::ports::SetDiagnosticsSink(request.diagnostics_sink);
  // Android keeps validation/runtime failures in the active UI diagnostics
  // channel and should not default to writing sidecar error-report files.
  tracer_core::domain::ports::SetErrorReportWriter(request.error_report_writer);
  // Reset session-level diagnostics state for this run.
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  tracer_core::domain::ports::ClearDiagnosticsDedup();

  auto pipeline_state = BuildAndroidPipelineState(
      kOutputRoot, kDbPath, kConverterConfigTomlPath);
  auto workflow = pipeline_state->workflow_handler;

  auto platform_clock =
      std::make_shared<infra_platform::AndroidPlatformClock>();

  auto insights_catalog = std::make_shared<InsightsCatalog>(
      android_runtime_detail::BuildAndroidInsightsCatalog(kOutputRoot,
                                                        kRuntimeConfigPaths));

  auto insights_query_service =
      std::make_unique<infra_insights::LazySqliteInsightsQueryService>(
          kDbPath, insights_catalog, platform_clock);
  auto insights =
      std::make_shared<InsightsHandler>(std::move(insights_query_service));

  auto project_repository =
      std::make_shared<infra_persistence_runtime::SqliteProjectRepository>(
          kDbPath.string());
  auto data_query_service =
      std::make_shared<tracer::core::infrastructure::query::data::repository::
                           QueryRuntimeService>(kDbPath,
                                                kConverterConfigTomlPath);
  auto insights_data_query_service =
      std::make_shared<infra_insights::LazySqliteInsightsDataQueryService>(
          kDbPath, platform_clock, insights_catalog);
  auto static_formatter_registrar = std::make_shared<
      infrastructure::insights::AndroidStaticInsightsFormatterRegistrar>(
      kRuntimeConfigPaths.formatter_policy);
  auto formatter_registry =
      tracer_core::application::ports::CreateInsightsFormatterRegistry(
          static_formatter_registrar);
  formatter_registry->RegisterFormatters();
  auto insights_dto_formatter =
      std::make_shared<infra_insights::InsightsDtoFormatter>(*insights_catalog);
  auto tracer_exchange_service =
      tracer_core::infrastructure::crypto::CreateTracerExchangeService(
          *workflow);

  // Runtime bootstrap owns capability composition; TracerCoreRuntime only
  // aggregates the already-constructed capability APIs.
  auto pipeline_api = std::make_shared<app_use_cases::PipelineApi>(*workflow);
  auto query_api = std::make_shared<app_use_cases::QueryApi>(
      project_repository, data_query_service);
  auto insights_api = std::make_shared<app_use_cases::InsightsApi>(
      *insights, insights_data_query_service, insights_dto_formatter);
  auto tracer_exchange_api = std::make_shared<app_use_cases::TracerExchangeApi>(
      tracer_exchange_service);

  AndroidRuntime runtime;
  auto runtime_impl = std::make_shared<app_use_cases::TracerCoreRuntime>(
      std::move(pipeline_api), std::move(query_api), std::move(insights_api),
      std::move(tracer_exchange_api));
  runtime.runtime_api = runtime_impl;

  auto runtime_state = std::make_shared<AndroidRuntimeState>();
  runtime_state->workflow_handler = std::move(workflow);
  runtime_state->insights_handler = std::move(insights);
  runtime_state->insights_catalog = std::move(insights_catalog);
  runtime.runtime_state = std::move(runtime_state);
  return runtime;
}

void SetAndroidRuntimeStatusConfigs(AndroidRuntime& runtime,
                                    InsightsStatusConfigs status_configs) {
  const auto state = std::static_pointer_cast<AndroidRuntimeState>(
      runtime.runtime_state);
  if (!state || !state->insights_catalog) {
    throw std::runtime_error("Android insights catalog is not initialized.");
  }
  state->insights_catalog->statuses = std::move(status_configs);
}

auto BuildAndroidPipelineRuntime(const AndroidRuntimeRequest& request)
    -> AndroidPipelineRuntime {
  const fs::path kOutputRoot =
      android_runtime_detail::ResolveOutputRoot(request.output_root);
  const fs::path kDbPath =
      android_runtime_detail::ResolveDbPath(request.db_path, kOutputRoot);
  const fs::path kConverterConfigTomlPath =
      android_runtime_detail::ResolveAndroidPipelineConfigPath(
          request.converter_config_toml_path);

  tracer_core::application::runtime_bridge::SetLogger(request.logger);
  tracer_core::domain::ports::SetDiagnosticsSink(request.diagnostics_sink);
  tracer_core::domain::ports::SetErrorReportWriter(request.error_report_writer);
  tracer_core::domain::ports::ClearBufferedDiagnostics();
  tracer_core::domain::ports::ClearDiagnosticsDedup();

  auto pipeline_state = BuildAndroidPipelineState(
      kOutputRoot, kDbPath, kConverterConfigTomlPath);
  AndroidPipelineRuntime runtime;
  runtime.pipeline_api = std::make_shared<app_use_cases::PipelineApi>(
      *pipeline_state->workflow_handler);
  runtime.runtime_state = std::move(pipeline_state);
  return runtime;
}

}  // namespace infrastructure::bootstrap
