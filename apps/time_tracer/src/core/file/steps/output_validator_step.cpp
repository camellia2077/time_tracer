// core/file/steps/output_validator_step.cpp
#include "output_validator_step.hpp"
#include "validator/json/facade/JsonValidator.hpp" 
#include "validator/common/ValidatorUtils.hpp"

#include "serializer/json_serializer.hpp"
#include "common/ansi_colors.hpp"
#include "io/core/file_reader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <set>
#include <nlohmann/json.hpp>

// [新增] 使用 validator 命名空间
using namespace validator;
using namespace validator::json;

bool OutputValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验数据逻辑 (JSON Logic) ---" << std::endl;

    bool has_memory_data = !context.result.processed_data.empty();
    bool has_generated_files = !context.state.generated_files.empty();

    if (!has_memory_data && !has_generated_files) {
        if (context.state.source_files.empty()) {
             std::cout << YELLOW_COLOR << "信息: 既无内存数据也无生成文件，跳过输出验证。" << RESET_COLOR << std::endl;
             return true;
        }
        std::cout << YELLOW_COLOR << "警告: 转换阶段成功完成，但未产生任何有效数据。" << RESET_COLOR << std::endl;
        return true; 
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    // [修改] 类现在在 namespace validator::json 中
    JsonValidator json_validator(context.config.date_check_mode); 

    if (has_memory_data) {
        std::cout << "正在验证内存中的转换数据..." << std::endl;
        
        for (const auto& [month_key, logs] : context.result.processed_data) {
            auto start_time = std::chrono::steady_clock::now();
            std::set<Error> errors; // [修改] Error 现在在 namespace validator 中

            try {
                nlohmann::json json_content = serializer::JsonSerializer::serializeDays(logs);
                std::string virtual_filename = "MemoryData_" + month_key; 
                
                if (!json_validator.validate(virtual_filename, json_content, errors)) {
                    printGroupedErrors(virtual_filename, errors); // [修改] 在 namespace validator 中
                    all_ok = false;
                }
            } catch (const std::exception& e) {
                std::cerr << RED_COLOR << "Error validating memory data for " << month_key << ": " << e.what() << RESET_COLOR << std::endl;
                all_ok = false;
            }
            auto end_time = std::chrono::steady_clock::now();
            total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
        }
    }
    else if (has_generated_files) {
        std::cout << "正在验证已生成的 JSON 文件..." << std::endl;
        for (const auto& file_to_validate : context.state.generated_files) {
            auto start_time = std::chrono::steady_clock::now();
            std::set<Error> errors;
            
            try {
                std::string content = FileReader::read_content(file_to_validate);
                nlohmann::json json_content = nlohmann::json::parse(content);

                if (!json_validator.validate(file_to_validate.string(), json_content, errors)) {
                    printGroupedErrors(file_to_validate.string(), errors);
                    all_ok = false;
                }
            } catch (const std::exception& e) {
                 std::cerr << RED_COLOR << "Error validating output file " << file_to_validate << ": " << e.what() << RESET_COLOR << std::endl;
                 all_ok = false;
            }
            auto end_time = std::chrono::steady_clock::now();
            total_ms += std::chrono::duration<double, std::milli>(end_time - start_time).count();
        }
    }

    printTiming(total_ms);
    if (!all_ok) {
        std::cout << RED_COLOR << "数据逻辑检验阶段 存在失败项。" << RESET_COLOR << std::endl;
        return false;
    }
    std::cout << GREEN_COLOR << "数据逻辑检验阶段 全部通过。" << RESET_COLOR << std::endl;
    return true;
}

void OutputValidatorStep::printTiming(double total_time_ms) const {
    double total_time_s = total_time_ms / 1000.0;
    std::cout << "--------------------------------------\n";
    std::cout << "检验耗时: " << std::fixed << std::setprecision(4) << total_time_s
              << " 秒 (" << total_time_ms << " ms)\n";
    std::cout << "--------------------------------------\n";
}