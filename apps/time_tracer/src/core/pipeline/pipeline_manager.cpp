// core/pipeline/pipeline_manager.cpp
#include "pipeline_manager.hpp"
#include "core/pipeline/steps/file_collector.hpp"
#include "core/pipeline/steps/source_validator_step.hpp"
#include "core/pipeline/steps/converter_step.hpp"
#include "core/pipeline/steps/output_validator_step.hpp"
#include "core/pipeline/utils/processed_data_writer.hpp"
#include "core/pipeline/utils/converter_config_factory.hpp" // [新增]
#include <iostream> 
#include "common/ansi_colors.hpp"

namespace core::pipeline {

PipelineManager::PipelineManager(const AppConfig& config, const fs::path& output_root)
    : app_config_(config), output_root_(output_root) {}

std::optional<PipelineContext> PipelineManager::run(const std::string& input_path, const AppOptions& options) {
    // 1. 初始化上下文
    PipelineContext context(app_config_, output_root_);
    context.config.input_root = input_path;
    context.config.date_check_mode = options.date_check_mode;
    context.config.save_processed_output = options.save_processed_output;

    std::cout << "\n=== Pipeline Start ===" << std::endl;

    // 2. 执行：收集文件
    FileCollector collector;
    if (!collector.execute(context, ".txt")) return std::nullopt;

    // [优化] 2.5 统一加载配置 (消除重复加载)
    // 无论后续步骤是否需要，提前加载配置通常是安全的且开销可控
    if (options.validate_source || options.convert) {
        try {
            std::cout << "Loading Configuration..." << std::endl;
            context.state.converter_config = ConverterConfigFactory::create(
                app_config_.pipeline.interval_processor_config_path,
                app_config_
            );
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "[Pipeline] 配置加载失败: " << e.what() << RESET_COLOR << std::endl;
            return std::nullopt;
        }
    }

    // 3. 执行：源文件验证 (直接使用 context 中的 config)
    if (options.validate_source) {
        SourceValidatorStep source_validator;
        if (!source_validator.execute(context)) {
            std::cerr << RED_COLOR << "[Pipeline] 源文件格式验证失败，终止流程。" << RESET_COLOR << std::endl;
            return std::nullopt;
        }
    }

    // 4. 执行：转换 (并行化处理，直接使用 context 中的 config)
    if (options.convert) {
        ConverterStep converter(app_config_);
        if (!converter.execute(context)) {
            std::cerr << RED_COLOR << "[Pipeline] 转换步骤存在错误，终止流程。" << RESET_COLOR << std::endl;
            return std::nullopt;
        }
    }

    // 5. 执行：输出逻辑验证 (同时缓存 JSON)
    if (options.validate_output) {
        OutputValidatorStep output_validator;
        if (!output_validator.execute(context)) {
            std::cerr << RED_COLOR << "[Pipeline] 逻辑验证发现错误，为防止脏数据入库，终止流程。" << RESET_COLOR << std::endl;
            return std::nullopt;
        }
    }

    // 6. 执行：保存 (使用缓存的 JSON)
    if (options.convert && options.save_processed_output) {
        std::cout << "Step: Saving Validated JSON..." << std::endl;
        auto new_files = ProcessedDataWriter::write(
            context.result.processed_data,
            context.cached_json_outputs, 
            context.config.output_root
        );
        context.state.generated_files.insert(
            context.state.generated_files.end(),
            new_files.begin(), 
            new_files.end()
        );
        
        if (new_files.empty() && !context.result.processed_data.empty()) {
             std::cerr << YELLOW_COLOR << "[Pipeline] 警告: 数据存在但未生成文件 (可能写入失败)。" << RESET_COLOR << std::endl;
        } else {
             std::cout << GREEN_COLOR << "JSON 数据已安全落盘。" << RESET_COLOR << std::endl;
        }
    }

    std::cout << "=== Pipeline Success ===" << std::endl;
    return context;
}

} // namespace core::pipeline