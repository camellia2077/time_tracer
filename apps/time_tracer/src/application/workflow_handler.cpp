// application/workflow_handler.cpp
#include "application/workflow_handler.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

#include "application/importer/data_importer.hpp"  // 包含 handle_process_memory_data
#include "application/pipeline/pipeline_manager.hpp"
#include "infrastructure/io/file_import_reader.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/serialization/json_serializer.hpp"  // 包含 JsonSerializer
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

// 使用之前定义的命名空间
using namespace core::pipeline;

WorkflowHandler::WorkflowHandler(std::string db_path, const AppConfig& config,
                                 fs::path output_root_path)
    : app_config_(config),
      db_path_(std::move(db_path)),
      output_root_path_(std::move(output_root_path)) {}

WorkflowHandler::~WorkflowHandler() = default;

auto WorkflowHandler::RunConverter(const std::string& input_path,
                                   const AppOptions& options) -> void {
  PipelineManager pipeline(app_config_, output_root_path_);
  if (!pipeline.Run(input_path, options)) {
    throw std::runtime_error("Converter Pipeline Failed.");
  }
}

auto WorkflowHandler::GetConfig() const -> const AppConfig& {
  return app_config_;
}

// [关键修复] 重写此函数：读取文件 -> 解析 JSON -> 传递 Struct 给 Importer
void WorkflowHandler::RunDatabaseImport(const std::string& processed_path_str) {
  // 1. 使用 Helper 读取文件内容
  auto import_payload =
      infrastructure::io::FileImportReader::ReadJsonFiles(processed_path_str);

  if (import_payload.empty()) {
    return;
  }

  std::cout << "正在解析 JSON 数据..." << std::endl;

  // 2. 在 Core 层完成解析 (JSON string -> DailyLog Struct)
  std::map<std::string, std::vector<DailyLog>> memory_data;

  for (const auto& [filepath, content] : import_payload) {
    try {
      auto json_obj = nlohmann::json::parse(content);
      std::vector<DailyLog> logs =
          serializer::JsonSerializer::DeserializeDays(json_obj);

      // 使用文件路径作为 key (仅用于日志/批次区分)
      memory_data[filepath] = logs;

    } catch (const std::exception& e) {
      std::cerr << time_tracer::common::colors::kRed << "解析文件失败 "
                << filepath << ": " << e.what()
                << time_tracer::common::colors::kReset << std::endl;
    }
  }

  if (memory_data.empty()) {
    std::cout << time_tracer::common::colors::kYellow
              << "没有有效的 JSON 数据可供导入。"
              << time_tracer::common::colors::kReset << std::endl;
    return;
  }

  // 3. 调用内存导入接口 (Importer 不再依赖 JSON)
  RunDatabaseImportFromMemory(memory_data);
}

auto WorkflowHandler::RunDatabaseImportFromMemory(
    const std::map<std::string, std::vector<DailyLog>>& data_map) -> void {
  HandleProcessMemoryData(db_path_, data_map);
}

auto WorkflowHandler::RunIngest(const std::string& source_path,
                                DateCheckMode date_check_mode,
                                bool save_processed) -> void {
  std::cout << "\n--- 启动数据摄入 (Ingest) ---" << std::endl;

  // RAII 检查数据库连接
  {
    DBManager db_manager(db_path_);
  }

  // 这里依然使用 PipelineManager，因为这是内部实现机制，没问题
  PipelineManager pipeline(app_config_, output_root_path_);

  AppOptions full_options;
  full_options.validate_structure = true;
  full_options.convert = true;
  full_options.validate_logic = true;
  full_options.date_check_mode = date_check_mode;
  full_options.save_processed_output = save_processed;

  auto result_context_opt = pipeline.Run(source_path, full_options);

  if (result_context_opt) {
    const auto& context = *result_context_opt;

    if (!context.result.processed_data.empty()) {
      std::cout << "\n--- 流水线验证通过，准备入库 ---" << std::endl;
      RunDatabaseImportFromMemory(context.result.processed_data);
      std::cout << time_tracer::common::colors::kGreen
                << "\n=== Ingest 执行成功 ==="
                << time_tracer::common::colors::kReset << std::endl;
    } else {
      std::cout << time_tracer::common::colors::kYellow
                << "\n=== Ingest 完成但无数据产生 ==="
                << time_tracer::common::colors::kReset << std::endl;
    }
  } else {
    std::cerr << time_tracer::common::colors::kRed
              << "\n=== Ingest 执行失败 ==="
              << time_tracer::common::colors::kReset << std::endl;
    throw std::runtime_error("Ingestion process failed.");
  }
}
