#include "OutputValidatorStep.hpp"
#include "reprocessing/validator/FileValidator.hpp"
#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <set>

bool OutputValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;

    // 确定要验证的文件列表
    const std::vector<fs::path>* files_ptr = &context.generated_files;
    
    // 如果没有生成新文件，尝试使用收集到的原始文件（针对仅运行 validate-output 的情况）
    if (context.generated_files.empty()) {
        if (!context.source_files.empty()) {
            files_ptr = &context.source_files;
        } else {
             std::cerr << YELLOW_COLOR << "警告: 没有找到可供检验的文件 (未生成新文件，且未收集到目标文件)。" << RESET_COLOR << std::endl;
             return true; 
        }
    }
    
    const auto& files_to_validate = *files_ptr;
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    FileValidator output_validator(context.config.interval_processor_config_path);

    for (const auto& file_to_validate : files_to_validate) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors;
        
        // 使用 context 中的 date_check_mode
        if (!output_validator.validate(file_to_validate.string(), ValidatorType::JsonOutput, errors, context.date_check_mode)) {
            printGroupedErrors(file_to_validate.string(), errors);
            all_ok = false;
        }
        auto end_time = std::chrono::steady_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    printTiming(total_ms);
    std::cout << (all_ok ? GREEN_COLOR : RED_COLOR) << "输出文件检验阶段 " 
              << (all_ok ? "全部通过" : "存在失败项") << "。" << RESET_COLOR << std::endl;
    return all_ok;
}

void OutputValidatorStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "Timing Statistics for: validateOutputFiles\n\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(4) << total_time_s
              << " seconds (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}