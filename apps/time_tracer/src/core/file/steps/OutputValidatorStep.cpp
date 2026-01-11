// core/file/steps/OutputValidatorStep.cpp
#include "OutputValidatorStep.hpp"
#include "converter/validator/FileValidator.hpp"
#include "converter/validator/common/ValidatorUtils.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <set>
#include <nlohmann/json.hpp>

bool OutputValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验输出文件 ---" << std::endl;

    // [修正] config.save_processed_output 和 state.generated_files
    if (!context.config.save_processed_output && context.state.generated_files.empty()) {
        std::cout << YELLOW_COLOR << "信息: 由于未保存中间 JSON 文件，跳过基于文件的输出验证。" << RESET_COLOR << std::endl;
        return true; 
    }

    const std::vector<fs::path>* files_ptr = &context.state.generated_files;
    if (context.state.generated_files.empty()) {
        if (!context.state.source_files.empty()) {
            files_ptr = &context.state.source_files;
        } else {
             std::cerr << YELLOW_COLOR << "警告: 没有找到可供检验的文件。" << RESET_COLOR << std::endl;
             return true; 
        }
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    // [修正] state.converter_config
    FileValidator output_validator(context.state.converter_config);

    for (const auto& file_to_validate : *files_ptr) {
        auto start_time = std::chrono::steady_clock::now();
        std::set<Error> errors;
        
        try {
            std::string content = FileReader::read_content(file_to_validate);
            nlohmann::json json_content = nlohmann::json::parse(content);

            // [修正] config.date_check_mode
            if (!output_validator.validate_json(file_to_validate.string(), json_content, errors, context.config.date_check_mode)) {
                printGroupedErrors(file_to_validate.string(), errors);
                all_ok = false;
            }
        } catch (const std::exception& e) {
             std::cerr << RED_COLOR << "Error validating output " << file_to_validate << ": " << e.what() << RESET_COLOR << std::endl;
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