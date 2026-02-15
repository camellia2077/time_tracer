#include "infrastructure/bootstrap/android_runtime_factory.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "application/interfaces/i_report_handler.hpp"
#include "application/interfaces/i_workflow_handler.hpp"
#include "application/ports/i_report_formatter_registry.hpp"
#include "application/ports/logger.hpp"
#include "application/reporting/report_handler.hpp"
#include "application/use_cases/time_tracer_core_api.hpp"
#include "application/workflow_handler.hpp"
#include "domain/ports/diagnostics.hpp"
#include "infrastructure/config/file_converter_config_provider.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/io/file_io_service.hpp"
#include "infrastructure/io/processed_data_io.hpp"
#include "infrastructure/io/txt_ingest_input_provider.hpp"
#include "infrastructure/logging/file_error_report_writer.hpp"
#include "infrastructure/persistence/importer/sqlite/connection.hpp"
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/persistence/sqlite_data_query_service.hpp"
#include "infrastructure/persistence/sqlite_database_health_checker.hpp"
#include "infrastructure/persistence/sqlite_time_sheet_repository.hpp"
#include "infrastructure/platform/android/android_platform_clock.hpp"
#include "infrastructure/reports/exporter.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"
#include "infrastructure/reports/report_dto_export_writer.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"
#include "infrastructure/reports/report_service.hpp"
#include "infrastructure/reports/sqlite_report_data_query_service.hpp"

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";

class SimpleStreamLogger final
    : public time_tracer::application::ports::ILogger {
 public:
  auto Log(time_tracer::application::ports::LogSeverity severity,
           std::string_view message) -> void override {
    std::scoped_lock lock(output_mutex_);
    std::ostream& output =
        severity == time_tracer::application::ports::LogSeverity::kError
            ? std::cerr
            : std::cout;
    output << "[" << ToLabel(severity) << "] " << message << '\n';
  }

 private:
  [[nodiscard]] static auto ToLabel(
      time_tracer::application::ports::LogSeverity severity)
      -> std::string_view {
    switch (severity) {
      case time_tracer::application::ports::LogSeverity::kInfo:
        return "INFO";
      case time_tracer::application::ports::LogSeverity::kWarn:
        return "WARN";
      case time_tracer::application::ports::LogSeverity::kError:
        return "ERROR";
    }
    return "INFO";
  }

  std::mutex output_mutex_;
};

class SimpleDiagnosticsSink final
    : public time_tracer::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(time_tracer::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override {
    std::scoped_lock lock(output_mutex_);
    std::ostream& output =
        severity == time_tracer::domain::ports::DiagnosticSeverity::kError
            ? std::cerr
            : std::cout;
    output << "[" << ToLabel(severity) << "] " << message << '\n';
  }

 private:
  [[nodiscard]] static auto ToLabel(
      time_tracer::domain::ports::DiagnosticSeverity severity)
      -> std::string_view {
    switch (severity) {
      case time_tracer::domain::ports::DiagnosticSeverity::kInfo:
        return "INFO";
      case time_tracer::domain::ports::DiagnosticSeverity::kWarn:
        return "WARN";
      case time_tracer::domain::ports::DiagnosticSeverity::kError:
        return "ERROR";
    }
    return "INFO";
  }

  std::mutex output_mutex_;
};

struct AndroidRuntimeState {
  std::shared_ptr<DBManager> db_manager;
  std::shared_ptr<IWorkflowHandler> workflow_handler;
  std::shared_ptr<IReportHandler> report_handler;
  std::shared_ptr<ReportCatalog> report_catalog;
};

[[nodiscard]] auto ResolveOutputRoot(const fs::path& output_root) -> fs::path {
  if (output_root.empty()) {
    throw std::invalid_argument(
        "Android runtime output_root must not be empty.");
  }
  return fs::absolute(output_root);
}

[[nodiscard]] auto ResolveDbPath(const fs::path& db_path,
                                 const fs::path& output_root) -> fs::path {
  if (!db_path.empty()) {
    return fs::absolute(db_path);
  }
  return fs::absolute(output_root / "db" / kDatabaseFilename);
}

auto EnsureDatabaseBootstrapped(const fs::path& db_path) -> void {
  fs::create_directories(db_path.parent_path());

  infrastructure::persistence::importer::sqlite::Connection bootstrap_db(
      db_path.string());
  if (bootstrap_db.GetDb() == nullptr) {
    throw std::runtime_error("Failed to initialize database at: " +
                             db_path.string());
  }
}

[[nodiscard]] auto ResolveConverterConfigTomlPath(
    const fs::path& converter_config_toml_path) -> fs::path {
  if (converter_config_toml_path.empty()) {
    throw std::invalid_argument(
        "Android runtime converter_config_toml_path must not be empty.");
  }

  const fs::path resolved = fs::absolute(converter_config_toml_path);
  if (!fs::exists(resolved)) {
    throw std::runtime_error("Converter config TOML not found: " +
                             resolved.string());
  }

  return resolved;
}

[[nodiscard]] auto ResolveReportConfigRoot(
    const fs::path& converter_config_toml_path) -> fs::path {
  const fs::path converter_dir = converter_config_toml_path.parent_path();
  if (converter_dir.filename() != "converter") {
    throw std::runtime_error(
        "Converter config path must be under 'config/"
        "converter': " +
        converter_config_toml_path.string());
  }

  const fs::path config_root = converter_dir.parent_path();
  if (config_root.empty() || !fs::exists(config_root)) {
    throw std::runtime_error("Config root not found for Android runtime: " +
                             config_root.string());
  }
  return config_root;
}

[[nodiscard]] auto BuildAndroidReportCatalog(const fs::path& output_root,
                                             const fs::path& config_root)
    -> ReportCatalog {
  ReportCatalog catalog;
  catalog.plugin_dir_path = output_root / "plugins";

  const fs::path markdown_dir = config_root / "reports" / "markdown";
  catalog.loaded_reports.markdown.day =
      ReportConfigLoader::LoadDailyMdConfig(markdown_dir / "day.toml");
  catalog.loaded_reports.markdown.month =
      ReportConfigLoader::LoadMonthlyMdConfig(markdown_dir / "month.toml");
  catalog.loaded_reports.markdown.period =
      ReportConfigLoader::LoadPeriodMdConfig(markdown_dir / "period.toml");
  catalog.loaded_reports.markdown.week =
      ReportConfigLoader::LoadWeeklyMdConfig(markdown_dir / "week.toml");
  catalog.loaded_reports.markdown.year =
      ReportConfigLoader::LoadYearlyMdConfig(markdown_dir / "year.toml");
  return catalog;
}

}  // namespace

namespace infrastructure::bootstrap {

auto BuildAndroidRuntime(const AndroidRuntimeRequest& request)
    -> AndroidRuntime {
  const fs::path kOutputRoot = ResolveOutputRoot(request.output_root);
  const fs::path kDbPath = ResolveDbPath(request.db_path, kOutputRoot);
  const fs::path kConverterConfigTomlPath =
      ResolveConverterConfigTomlPath(request.converter_config_toml_path);
  const fs::path kReportConfigRoot =
      ResolveReportConfigRoot(kConverterConfigTomlPath);

  FileIoService::PrepareOutputDirectories(kOutputRoot);
  EnsureDatabaseBootstrapped(kDbPath);

  time_tracer::application::ports::SetLogger(
      request.logger ? request.logger : std::make_shared<SimpleStreamLogger>());

  time_tracer::domain::ports::SetDiagnosticsSink(
      request.diagnostics_sink ? request.diagnostics_sink
                               : std::make_shared<SimpleDiagnosticsSink>());
  time_tracer::domain::ports::SetErrorReportWriter(
      request.error_report_writer
          ? request.error_report_writer
          : std::make_shared<infrastructure::logging::FileErrorReportWriter>(
                kOutputRoot / "errors.log"));

  auto db_manager = std::make_shared<DBManager>(kDbPath.string());
  if (!db_manager->OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database at: " + kDbPath.string());
  }
  sqlite3* db_connection = db_manager->GetDbConnection();
  if (db_connection == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }

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

  auto workflow = std::make_shared<WorkflowHandler>(
      kOutputRoot, std::move(processed_data_loader),
      std::move(time_sheet_repository), std::move(database_health_checker),
      std::move(converter_config_provider), std::move(ingest_input_provider),
      std::move(processed_data_storage));

  auto platform_clock =
      std::make_shared<infrastructure::platform::AndroidPlatformClock>();

  auto report_catalog = std::make_shared<ReportCatalog>(
      BuildAndroidReportCatalog(kOutputRoot, kReportConfigRoot));

  auto report_query_service = std::make_unique<ReportService>(
      db_connection, *report_catalog, platform_clock);
  auto exporter = std::make_unique<Exporter>(kOutputRoot);
  auto report = std::make_shared<ReportHandler>(std::move(report_query_service),
                                                std::move(exporter));

  auto project_repository =
      std::make_shared<SqliteProjectRepository>(kDbPath.string());
  auto data_query_service =
      std::make_shared<infrastructure::persistence::SqliteDataQueryService>(
          kDbPath);
  auto report_data_query_service =
      std::make_shared<infrastructure::reports::SqliteReportDataQueryService>(
          db_connection, platform_clock);
  auto static_formatter_registrar = std::make_shared<
      infrastructure::reports::AndroidStaticReportFormatterRegistrar>();
  auto formatter_registry =
      time_tracer::application::ports::CreateReportFormatterRegistry(
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
  runtime.core_api = std::make_shared<TimeTracerCoreApi>(
      *workflow, *report, project_repository, std::move(data_query_service),
      std::move(report_data_query_service), std::move(report_dto_formatter),
      std::move(report_export_writer));

  auto runtime_state = std::make_shared<AndroidRuntimeState>();
  runtime_state->db_manager = std::move(db_manager);
  runtime_state->workflow_handler = std::move(workflow);
  runtime_state->report_handler = std::move(report);
  runtime_state->report_catalog = std::move(report_catalog);
  runtime.runtime_state = std::move(runtime_state);
  return runtime;
}

}  // namespace infrastructure::bootstrap
