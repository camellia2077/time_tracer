// application/pipeline/pipeline_manager.cpp
#include "pipeline_manager.hpp"

#include <iostream>
#include <utility>

#include "common/ansi_colors.hpp"
#include "application/pipeline/steps/converter_step.hpp"
#include "application/pipeline/steps/file_collector.hpp"
#include "application/pipeline/steps/logic_linker_step.hpp"
#include "application/pipeline/steps/logic_validator_step.hpp"
#include "application/pipeline/steps/structure_validator_step.hpp"
#include "application/pipeline/utils/converter_config_factory.hpp"
#include "infrastructure/io/processed_data_writer.hpp"

namespace core::pipeline {

PipelineManager::PipelineManager(const AppConfig& config, fs::path output_root)
    : app_config_(config), output_root_(std::move(output_root)) {}

auto PipelineManager::run(const std::string& input_path,
                          const AppOptions& options)
    -> std::optional<PipelineContext> {
  // 1. 初始化上下文
  PipelineContext context(app_config_, output_root_);
  context.config.input_root = input_path;
  context.config.date_check_mode = options.date_check_mode;
  context.config.save_processed_output = options.save_processed_output;

  std::cout << "\n=== Pipeline Start ===" << std::endl;

  // 2. 执行：收集文件
  FileCollector collector;
  if (!collector.execute(context, ".txt")) {
    return std::nullopt;
  }

  // 2.5 统一加载配置
  if (options.validate_structure || options.convert) {
    try {
      std::cout << "Loading Configuration..." << std::endl;
      context.state.converter_config = ConverterConfigFactory::create(
          app_config_.pipeline.interval_processor_config_path, app_config_);
    } catch (const std::exception& e) {
      std::cerr << RED_COLOR << "[Pipeline] 配置加载失败: " << e.what()
                << RESET_COLOR << std::endl;
      return std::nullopt;
    }
  }

  // 3. 执行：结构验证 (Structure Validation)
  if (options.validate_structure) {
    std::cout << "Step: Validating File Structure..." << std::endl;
    StructureValidatorStep structure_validator;
    if (!structure_validator.execute(context)) {
      // 结构错误通常意味着无法解析，直接终止
      return std::nullopt;
    }
  }

  // 4. 执行：转换
  if (options.convert) {
    // 4.1 并行文件转换 (文件内逻辑)
    ConverterStep converter(app_config_);
    if (!converter.execute(context)) {
      std::cerr << RED_COLOR << "[Pipeline] 转换步骤存在错误，终止流程。"
                << RESET_COLOR << std::endl;
      return std::nullopt;
    }

    // [核心修复] 4.2 执行逻辑链接 (跨文件/跨月逻辑)
    // 必须在 ConverterStep 收集完所有数据后执行
    LogicLinkerStep logic_linker;
    if (!logic_linker.execute(context)) {
      std::cerr << RED_COLOR << "[Pipeline] 跨月数据连接失败，流程终止。"
                << RESET_COLOR << std::endl;
      return std::nullopt;
    }
  }

  // [重命名] 5. 执行：逻辑验证 (Logic Validation)
  if (options.validate_logic) {
    std::cout << "Step: Validating Business Logic..." << std::endl;
    LogicValidatorStep logic_validator;
    if (!logic_validator.execute(context)) {
      // 逻辑错误可能影响入库，终止
      return std::nullopt;
    }
  }

  // 6. 执行：保存
  if (options.convert && options.save_processed_output) {
    std::cout << "Step: Saving Validated JSON..." << std::endl;
    auto new_files = infrastructure::io::ProcessedDataWriter::write(context.result.processed_data,
                                                context.cached_json_outputs,
                                                context.config.output_root);
    context.state.generated_files.insert(context.state.generated_files.end(),
                                         new_files.begin(), new_files.end());

    if (new_files.empty() && !context.result.processed_data.empty()) {
      std::cerr << YELLOW_COLOR
                << "[Pipeline] 警告: 数据存在但未生成文件 (可能写入失败)。"
                << RESET_COLOR << std::endl;
    } else {
      std::cout << GREEN_COLOR << "JSON 数据已安全落盘。" << RESET_COLOR
                << std::endl;
    }
  }

  std::cout << "=== Pipeline Success ===" << std::endl;
  return context;
}

}  // namespace core::pipeline
