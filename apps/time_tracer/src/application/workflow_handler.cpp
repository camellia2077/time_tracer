// application/workflow_handler.cpp
#include "application/workflow_handler.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "application/importer/import_service.hpp"
#include "application/pipeline/pipeline_manager.hpp"
#include "application/ports/logger.hpp"
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

// 使用之前定义的命名空间
using namespace core::pipeline;
namespace app_ports = time_tracer::application::ports;

namespace {

auto PrintImportStats(const ImportStats& stats, std::string_view title)
    -> void {
  std::ostringstream stream;
  stream << "\n--- " << title << " Report ---";
  app_ports::LogInfo(stream.str());

  namespace colors = time_tracer::common::colors;
  if (!stats.db_open_success) {
    app_ports::LogError(
        std::string(colors::kRed) + "[Fatal] DB Error: " +
        (stats.error_message.empty() ? "Unknown" : stats.error_message) +
        colors::kReset.data());
    return;
  }

  if (!stats.transaction_success) {
    app_ports::LogError(std::string(colors::kRed) +
                        "[Fatal] Transaction Failed: " + stats.error_message +
                        colors::kReset.data());
    return;
  }

  if (stats.failed_files.empty()) {
    app_ports::LogInfo(std::string(colors::kGreen) + "[Success] Processed " +
                       std::to_string(stats.successful_files) + " items." +
                       colors::kReset.data());
  } else {
    app_ports::LogWarn(std::string(colors::kYellow) + "[Partial] Success: " +
                       std::to_string(stats.successful_files) + ", Failed: " +
                       std::to_string(stats.failed_files.size()) +
                       colors::kReset.data());

    for (const auto& failed_file : stats.failed_files) {
      app_ports::LogError("  Failed: " + failed_file);
    }
  }

  const double kTotalTime =
      stats.parsing_duration_s + stats.db_insertion_duration_s;

  std::ostringstream timing;
  timing << std::fixed << std::setprecision(3)
         << "Timing: Parse=" << stats.parsing_duration_s
         << "s, Insert=" << stats.db_insertion_duration_s
         << "s, Total=" << kTotalTime << "s";
  app_ports::LogInfo(timing.str());
}

}  // namespace

WorkflowHandler::WorkflowHandler(
    fs::path output_root_path,
    std::shared_ptr<app_ports::IProcessedDataLoader> processed_data_loader,
    std::shared_ptr<app_ports::ITimeSheetRepository> time_sheet_repository,
    std::shared_ptr<app_ports::IDatabaseHealthChecker> database_health_checker,
    std::shared_ptr<app_ports::IConverterConfigProvider>
        converter_config_provider,
    std::shared_ptr<app_ports::IIngestInputProvider> ingest_input_provider,
    std::shared_ptr<app_ports::IProcessedDataStorage> processed_data_storage)
    : output_root_path_(std::move(output_root_path)),
      processed_data_loader_(std::move(processed_data_loader)),
      time_sheet_repository_(std::move(time_sheet_repository)),
      database_health_checker_(std::move(database_health_checker)),
      converter_config_provider_(std::move(converter_config_provider)),
      ingest_input_provider_(std::move(ingest_input_provider)),
      processed_data_storage_(std::move(processed_data_storage)) {
  if (!processed_data_loader_ || !time_sheet_repository_ ||
      !database_health_checker_ || !converter_config_provider_ ||
      !ingest_input_provider_ || !processed_data_storage_) {
    throw std::invalid_argument(
        "WorkflowHandler dependencies must not be null.");
  }
}

WorkflowHandler::~WorkflowHandler() = default;

auto WorkflowHandler::RunConverter(const std::string& input_path,
                                   const AppOptions& options) -> void {
  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_);
  if (!pipeline.Run(input_path, options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
  app_ports::LogInfo("正在解析 JSON 数据...");

  auto load_result = processed_data_loader_->LoadDailyLogs(processed_path_str);

  namespace colors = time_tracer::common::colors;
  for (const auto& error : load_result.errors) {
    app_ports::LogError(std::string(colors::kRed) + "解析文件失败 " +
                        error.source + ": " + error.message +
                        colors::kReset.data());
  }

  if (load_result.data_by_source.empty()) {
    app_ports::LogWarn(std::string(colors::kYellow) +
                       "没有有效的 JSON 数据可供导入。" +
                       colors::kReset.data());
    return;
  }

  RunDatabaseImportFromMemory(load_result.data_by_source);
}

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  app_ports::LogInfo("Task: Memory Import...");

  ImportService service(*time_sheet_repository_);
  ImportStats stats = service.ImportFromMemory(data_map);

  PrintImportStats(stats, "Memory Import");

  if (!stats.db_open_success || !stats.transaction_success) {
    throw std::runtime_error(stats.error_message.empty()
                                 ? "Memory import failed."
                                 : stats.error_message);
  }
}

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed) -> void {
  app_ports::LogInfo("\n--- 启动数据摄入 (Ingest) ---");

  const auto kDbCheck = database_health_checker_->CheckReady();
  if (!kDbCheck.ok) {
    throw std::runtime_error(kDbCheck.message.empty()
                                 ? "Database readiness check failed."
                                 : kDbCheck.message);
  }

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_);

  AppOptions full_options;
  full_options.validate_structure = true;
  full_options.convert = true;
  full_options.validate_logic = true;
  full_options.date_check_mode = date_check_mode;
  full_options.save_processed_output = save_processed;

  auto result_context_opt = pipeline.Run(source_path, full_options);

  namespace colors = time_tracer::common::colors;
  if (result_context_opt) {
    const auto& context = *result_context_opt;

    if (!context.result.processed_data.empty()) {
      app_ports::LogInfo("\n--- 流水线验证通过，准备入库 ---");
      RunDatabaseImportFromMemory(context.result.processed_data);
      app_ports::LogInfo(std::string(colors::kGreen) +
                         "\n=== Ingest 执行成功 ===" + colors::kReset.data());
    } else {
      app_ports::LogWarn(
          std::string(colors::kYellow) +
          "\n=== Ingest 完成但无数据产生 ===" + colors::kReset.data());
    }
    return;
  }

  app_ports::LogError(std::string(colors::kRed) +
                      "\n=== Ingest 执行失败 ===" + colors::kReset.data());
  throw std::runtime_error("Ingestion process failed.");
}

auto WorkflowHandler::RunValidateStructure(const std::string& source_path)
    -> void {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = true;
  options.convert = false;
  options.validate_logic = false;
  options.save_processed_output = false;
  options.date_check_mode = DateCheckMode::kNone;

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_);
  if (!pipeline.Run(source_path, options)) {
    throw std::runtime_error("Validate structure pipeline failed.");
  }
}

auto WorkflowHandler::RunValidateLogic(const std::string& source_path,
                                       DateCheckMode date_check_mode) -> void {
  AppOptions options;
  options.input_path = source_path;
  options.validate_structure = false;
  options.convert = true;
  options.validate_logic = true;
  options.save_processed_output = false;
  options.date_check_mode = date_check_mode;

  PipelineManager pipeline(output_root_path_, converter_config_provider_,
                           ingest_input_provider_, processed_data_storage_);
  if (!pipeline.Run(source_path, options)) {
    throw std::runtime_error("Validate logic pipeline failed.");
  }
}
