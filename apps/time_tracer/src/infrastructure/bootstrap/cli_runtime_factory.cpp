// infrastructure/bootstrap/cli_runtime_factory.cpp
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/bootstrap/startup_validator.hpp"
#include "application/dto/runtime_environment_requirements.hpp"
#include "application/interfaces/i_report_handler.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_cli_runtime_factory.hpp"
#include "application/ports/i_report_formatter_registry.hpp"
#include "application/ports/logger.hpp"
#include "application/reporting/report_handler.hpp"
#include "application/use_cases/time_tracer_core_api.hpp"
#include "application/workflow_handler.hpp"
#include "domain/ports/diagnostics.hpp"
#include "infrastructure/config/config_loader.hpp"
#include "infrastructure/config/file_converter_config_provider.hpp"
#include "infrastructure/config/models/app_config.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/config/validator/plugins/facade/runtime_environment_validator_adapter.hpp"
#include "infrastructure/io/file_io_service.hpp"
#include "infrastructure/io/processed_data_io.hpp"
#include "infrastructure/io/txt_ingest_input_provider.hpp"
#include "infrastructure/logging/console_diagnostics_sink.hpp"
#include "infrastructure/logging/console_logger.hpp"
#include "infrastructure/logging/file_error_report_writer.hpp"
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/persistence/sqlite_data_query_service.hpp"
#include "infrastructure/persistence/sqlite_database_health_checker.hpp"
#include "infrastructure/persistence/sqlite_time_sheet_repository.hpp"
#include "infrastructure/platform/windows/windows_platform_clock.hpp"
#include "infrastructure/reports/exporter.hpp"
#include "infrastructure/reports/plugin_manifest.hpp"
#include "infrastructure/reports/report_dto_export_writer.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"
#include "infrastructure/reports/report_service.hpp"
#include "infrastructure/reports/sqlite_report_data_query_service.hpp"

namespace {

namespace fs = std::filesystem;
constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";

auto ToCliConfig(const AppConfig& app_config)
    -> time_tracer::application::dto::CliConfig {
  time_tracer::application::dto::CliConfig config;
  config.default_save_processed_output =
      app_config.default_save_processed_output;
  config.default_date_check_mode = app_config.default_date_check_mode;
  config.defaults.default_format = app_config.defaults.default_format;
  config.command_defaults.export_format =
      app_config.command_defaults.export_format;
  config.command_defaults.query_format =
      app_config.command_defaults.query_format;
  config.command_defaults.convert_date_check_mode =
      app_config.command_defaults.convert_date_check_mode;
  config.command_defaults.convert_save_processed_output =
      app_config.command_defaults.convert_save_processed_output;
  config.command_defaults.convert_validate_logic =
      app_config.command_defaults.convert_validate_logic;
  config.command_defaults.convert_validate_structure =
      app_config.command_defaults.convert_validate_structure;
  config.command_defaults.ingest_date_check_mode =
      app_config.command_defaults.ingest_date_check_mode;
  config.command_defaults.ingest_save_processed_output =
      app_config.command_defaults.ingest_save_processed_output;
  config.command_defaults.validate_logic_date_check_mode =
      app_config.command_defaults.validate_logic_date_check_mode;
  return config;
}

auto BuildReportCatalog(const AppConfig& app_config) -> ReportCatalog {
  ReportCatalog catalog;
  catalog.plugin_dir_path = fs::path(app_config.exe_dir_path) / "plugins";
  catalog.loaded_reports = app_config.loaded_reports;
  return catalog;
}

auto ResolveOutputRoot(const AppConfig& app_config, const fs::path& exe_path)
    -> fs::path {
  fs::path output_root = fs::absolute(exe_path.parent_path() / "output");
  if (app_config.defaults.output_root.has_value()) {
    output_root = fs::absolute(*app_config.defaults.output_root);
  }
  return output_root;
}

auto ResolveDbPath(const AppConfig& app_config, const fs::path& output_root,
                   const std::optional<fs::path>& db_override) -> fs::path {
  if (db_override.has_value()) {
    return fs::absolute(*db_override);
  }

  fs::path default_db_path = output_root / "db" / kDatabaseFilename;
  if (app_config.defaults.kDbPath.has_value()) {
    default_db_path = fs::absolute(*app_config.defaults.kDbPath);
  }
  return default_db_path;
}

auto ResolveExportPath(const AppConfig& app_config,
                       const std::optional<fs::path>& output_override)
    -> fs::path {
  if (output_override.has_value()) {
    return fs::absolute(*output_override);
  }
  if (app_config.kExportPath.has_value()) {
    return fs::absolute(*app_config.kExportPath);
  }
  throw std::runtime_error(
      "Missing export output directory. Provide --output <dir> or configure "
      "[system].export_root in config/config.toml.");
}

auto NeedsOpenDb(std::string_view command_name) -> bool {
  return command_name == "query" || command_name == "export";
}

struct CliRuntimeState {
  std::shared_ptr<DBManager> db_manager;
  std::shared_ptr<IWorkflowHandler> workflow_handler;
  std::shared_ptr<IReportHandler> report_handler;
  std::shared_ptr<ReportCatalog> report_catalog;
  std::shared_ptr<AppConfig> app_config;
};

class InfrastructureCliRuntimeFactory final
    : public time_tracer::application::ports::ICliRuntimeFactory {
 public:
  [[nodiscard]] auto ValidateEnvironment(const fs::path& executable_path,
                                         bool is_help_mode) const
      -> bool override {
    ConfigLoader loader(executable_path.string());
    const AppConfig kConfig = loader.LoadConfiguration();

    if (is_help_mode) {
      return true;
    }

    time_tracer::application::dto::RuntimeEnvironmentRequirements requirements;
    requirements.plugins_directory = fs::path(kConfig.exe_dir_path) / "plugins";
    requirements.binary_directory = kConfig.exe_dir_path;
    auto formatter_registry =
        time_tracer::application::ports::CreateReportFormatterRegistry();
    requirements.expected_formatter_plugins =
        formatter_registry->GetExpectedFormatterPluginNames();
    requirements.required_core_runtime_libraries = {
        std::string(reports::plugin_manifest::kCoreRuntimeLibraryName)};

    infrastructure::config::validator::plugins::
        RuntimeEnvironmentValidatorAdapter validator;
    return StartupValidator::ValidateEnvironment(requirements, validator);
  }

  [[nodiscard]] auto BuildRuntime(
      const time_tracer::application::ports::CliRuntimeRequest& request) const
      -> time_tracer::application::ports::CliRuntime override {
    using time_tracer::application::ports::CliRuntime;

    ConfigLoader config_loader(request.executable_path.string());
    auto app_config =
        std::make_shared<AppConfig>(config_loader.LoadConfiguration());
    auto report_catalog =
        std::make_shared<ReportCatalog>(BuildReportCatalog(*app_config));

    time_tracer::application::ports::SetLogger(
        std::make_shared<infrastructure::logging::ConsoleLogger>());

    const fs::path kOutputRootPath =
        ResolveOutputRoot(*app_config, request.executable_path);
    const fs::path kDbPath =
        ResolveDbPath(*app_config, kOutputRootPath, request.db_override);
    const fs::path kExportPath =
        ResolveExportPath(*app_config, request.output_override);

    auto diagnostics_sink =
        std::make_shared<infrastructure::logging::ConsoleDiagnosticsSink>();
    auto error_writer =
        std::make_shared<infrastructure::logging::FileErrorReportWriter>(
            kOutputRootPath / "errors.log");
    time_tracer::domain::ports::SetDiagnosticsSink(diagnostics_sink);
    time_tracer::domain::ports::SetErrorReportWriter(error_writer);

    FileIoService::PrepareOutputDirectories(kOutputRootPath);

    auto db_manager = std::make_shared<DBManager>(kDbPath.string());
    if (NeedsOpenDb(request.command_name) &&
        !db_manager->OpenDatabaseIfNeeded()) {
      throw std::runtime_error(
          "Failed to open database at: " + kDbPath.string() +
          "\nPlease ensure data has been imported or check the path.");
    }
    sqlite3* db_connection = db_manager->GetDbConnection();

    auto processed_data_loader =
        std::make_shared<infrastructure::io::ProcessedDataLoader>();
    auto time_sheet_repository = std::make_shared<
        infrastructure::persistence::SqliteTimeSheetRepository>(
        kDbPath.string());
    auto database_health_checker = std::make_shared<
        infrastructure::persistence::SqliteDatabaseHealthChecker>(
        kDbPath.string());
    auto converter_config_provider =
        std::make_shared<infrastructure::config::FileConverterConfigProvider>(
            app_config->pipeline.interval_processor_config_path,
            app_config->pipeline.initial_top_parents);
    static_cast<void>(converter_config_provider->LoadConverterConfig());
    auto ingest_input_provider =
        std::make_shared<infrastructure::io::TxtIngestInputProvider>();
    auto processed_data_storage =
        std::make_shared<infrastructure::io::ProcessedDataStorage>();

    auto workflow = std::make_shared<WorkflowHandler>(
        kOutputRootPath, std::move(processed_data_loader),
        std::move(time_sheet_repository), std::move(database_health_checker),
        std::move(converter_config_provider), std::move(ingest_input_provider),
        std::move(processed_data_storage));

    auto formatter_registry =
        time_tracer::application::ports::CreateReportFormatterRegistry();
    formatter_registry->RegisterFormatters();
    auto platform_clock =
        std::make_shared<infrastructure::platform::WindowsPlatformClock>();
    auto report_query_service = std::make_unique<ReportService>(
        db_connection, *report_catalog, platform_clock);
    auto exporter = std::make_unique<Exporter>(kExportPath);
    auto report = std::make_shared<ReportHandler>(
        std::move(report_query_service), std::move(exporter));

    auto project_repository =
        std::make_shared<SqliteProjectRepository>(kDbPath.string());
    auto data_query_service =
        std::make_shared<infrastructure::persistence::SqliteDataQueryService>(
            kDbPath);
    auto report_data_query_service =
        std::make_shared<infrastructure::reports::SqliteReportDataQueryService>(
            db_connection, platform_clock);
    auto report_dto_formatter =
        std::make_shared<infrastructure::reports::ReportDtoFormatter>(
            *report_catalog);
    auto report_exporter_for_dto = std::make_shared<Exporter>(kExportPath);
    auto report_export_writer =
        std::make_shared<infrastructure::reports::ReportDtoExportWriter>(
            report_dto_formatter, report_exporter_for_dto);

    CliRuntime runtime;
    runtime.core_api = std::make_shared<TimeTracerCoreApi>(
        *workflow, *report, project_repository, std::move(data_query_service),
        std::move(report_data_query_service), std::move(report_dto_formatter),
        std::move(report_export_writer));
    runtime.cli_config = ToCliConfig(*app_config);
    auto runtime_state = std::make_shared<CliRuntimeState>();
    runtime_state->db_manager = std::move(db_manager);
    runtime_state->workflow_handler = workflow;
    runtime_state->report_handler = report;
    runtime_state->report_catalog = std::move(report_catalog);
    runtime_state->app_config = std::move(app_config);
    runtime.runtime_state = std::move(runtime_state);
    return runtime;
  }
};

}  // namespace

namespace time_tracer::application::ports {

auto CreateCliRuntimeFactory() -> std::shared_ptr<ICliRuntimeFactory> {
  return std::make_shared<InfrastructureCliRuntimeFactory>();
}

}  // namespace time_tracer::application::ports
