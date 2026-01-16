// core/file/steps/SourceValidatorStep.cpp
#include "SourceValidatorStep.hpp"
#include "core/file/utils/ConverterConfigFactory.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

// [路径修正] 不再需要包含 FileValidator，因为验证逻辑已移至 LogProcessor (如果 LogProcessor 内部处理)
// 或者如果 SourceValidatorStep 显式调用 TextValidator，则需包含
// 根据你之前的代码，Processor 负责处理。但为了保持一致性，如果这里将来直接调用 Validator，路径应为:
// #include "validator/FileValidator.hpp" 

// 目前 LogProcessor 负责转换，但在 SourceValidatorStep 中我们实际上应该调用 TextValidator
// 让我们查看原有代码，它调用了 LogProcessor.processSourceContent。
// 在之前的重构中，LogProcessor 移除了验证逻辑。
// 所以 SourceValidatorStep 必须直接调用 TextValidator！

#include "validator/source_txt/facade/TextValidator.hpp" // [新增] 直接调用验证器

bool SourceValidatorStep::execute(PipelineContext& context) {
    std::cout << "Step: Validating source files..." << std::endl;

    // 1. 加载配置
    ConverterConfig config;
    try {
        config = ConverterConfigFactory::create(
            context.config.app_config.pipeline.interval_processor_config_path,
            context.config.app_config
        );
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "验证准备阶段配置加载失败: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    // [核心修改] 直接使用 TextValidator 而不是 LogProcessor
    TextValidator validator(config);

    // 2. 执行验证
    for (const auto& file_path : context.state.source_files) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors; // [新增]

        try {
            std::string content = FileReader::read_content(file_path);
            
            // 调用 TextValidator
            if (!validator.validate(file_path.string(), content, errors)) {
                printGroupedErrors(file_path.string(), errors); // 使用 ValidatorUtils 中的打印函数
                all_ok = false;
            }
            
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "IO Error validating " << file_path << ": " << e.what() << RESET_COLOR << std::endl;
            all_ok = false;
        }
        
        auto end_time = std::chrono::steady_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    printTiming(total_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "源文件检验阶段 " 
              << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

void SourceValidatorStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "检验耗时: " << std::fixed << std::setprecision(3) << total_time_s << " 秒 (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}
