// core/file/PipelineManager.cpp
#include "PipelineManager.hpp"
// PipelineContext.hpp 已在头文件包含，但如果为了保险可以再包含
// #include "core/file/PipelineContext.hpp" 
#include "steps/FileCollector.hpp"
#include "steps/SourceValidatorStep.hpp"
#include "steps/ConverterStep.hpp"
#include "steps/OutputValidatorStep.hpp"
#include <iostream> 
#include "common/AnsiColors.hpp"

PipelineManager::PipelineManager(const AppConfig& config, const fs::path& output_root)
    : app_config_(config), output_root_(output_root) {}

std::optional<PipelineContext> PipelineManager::run(const std::string& input_path, const AppOptions& options) {
    // 1. 初始化上下文
    PipelineContext context(app_config_, output_root_);
    context.input_root = input_path;
    context.date_check_mode = options.date_check_mode;
    context.save_processed_output = options.save_processed_output; // [新增] 传递参数

    // 2. 确定要收集的文件扩展名
    std::string extension = ".txt";
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
    if (options.validate_output) {
        OutputValidatorStep output_validator;
        if (!output_validator.execute(context)) return std::nullopt;
    }

    // [核心修改] 返回整个 context
    return context;
}