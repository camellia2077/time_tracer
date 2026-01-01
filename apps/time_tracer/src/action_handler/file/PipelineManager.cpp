// action_handler/file/PipelineManager.cpp
#include "PipelineManager.hpp"
#include "action_handler/file/PipelineContext.hpp"
#include "steps/FileCollector.hpp"
#include "steps/SourceValidatorStep.hpp"
#include "steps/ConverterStep.hpp"
#include "steps/OutputValidatorStep.hpp"
#include <iostream> 
#include "common/AnsiColors.hpp"

PipelineManager::PipelineManager(const AppConfig& config, const fs::path& output_root)
    : app_config_(config), output_root_(output_root) {}

// [核心修改] 根据 options 动态决定执行哪些步骤
std::optional<fs::path> PipelineManager::run(const std::string& input_path, const AppOptions& options) {
    // 1. 初始化上下文
    PipelineContext context(app_config_, output_root_);
    context.input_root = input_path;
    context.date_check_mode = options.date_check_mode;

    // 2. 确定要收集的文件扩展名
    std::string extension = ".txt";
    // 逻辑复刻自原本的 FileHandler：如果只验证输出且不转换，则收集 .json
    if (options.validate_output && !options.convert && !options.validate_source) {
        extension = ".json";
    }

    // 3. 执行：收集文件
    FileCollector collector;
    if (!collector.execute(context, extension)) return std::nullopt;

    // 4. 执行：源文件验证 (按需)
    if (options.validate_source) {
        SourceValidatorStep source_validator;
        if (!source_validator.execute(context)) return std::nullopt;
    }

    // 5. 执行：转换 (按需)
    if (options.convert) {
        ConverterStep converter(app_config_);
        if (!converter.execute(context)) return std::nullopt;
    }

    // 6. 执行：输出文件验证 (按需)
    // OutputValidatorStep 内部已经有逻辑：如果没有生成新文件(未转换)，则验证收集到的源文件
    if (options.validate_output) {
        OutputValidatorStep output_validator;
        if (!output_validator.execute(context)) return std::nullopt;
    }

    return context.output_root / "Processed_Date";
}