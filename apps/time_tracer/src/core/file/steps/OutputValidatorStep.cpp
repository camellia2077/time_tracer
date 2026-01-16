// core/file/steps/OutputValidatorStep.cpp
#include "OutputValidatorStep.hpp"
#include "validator/FileValidator.hpp"
#include "validator/common/ValidatorUtils.hpp"
#include "serializer/JsonSerializer.hpp"
#include "common/AnsiColors.hpp"
#include "io/core/FileReader.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <set>
#include <nlohmann/json.hpp>

bool OutputValidatorStep::execute(PipelineContext& context) {
    std::cout << "\n--- 阶段: 检验数据逻辑 (JSON Logic) ---" << std::endl;

    // 1. 检查内存中是否有数据 (Blink 模式的核心)
    bool has_memory_data = !context.result.processed_data.empty();
    
    // 2. 检查是否有已生成的文件 (非 Blink 模式，或已落盘模式)
    bool has_generated_files = !context.state.generated_files.empty();

    // 异常情况：既没内存数据，也没生成文件
    if (!has_memory_data && !has_generated_files) {
        // 如果原本就是去验证现有文件(validate-output模式)，且没找到文件
        if (context.state.source_files.empty()) {
             std::cout << YELLOW_COLOR << "信息: 既无内存数据也无生成文件，跳过输出验证。" << RESET_COLOR << std::endl;
             return true;
        }
        
        // 如果有源文件，但到了这一步还没数据，说明转换失败了或者被过滤了
        std::cout << YELLOW_COLOR << "警告: 转换阶段成功完成，但未产生任何有效数据 (processed_data 为空)。" << RESET_COLOR << std::endl;
        std::cout << "这可能是因为源文件内容为空，或被过滤器排除了。" << std::endl;
        return true; 
    }
    
    bool all_ok = true;
    double total_ms = 0.0;
    
    FileValidator output_validator(context.state.converter_config);

    // -------------------------------------------------------
    // 路径 A: 内存数据验证 (Blink 模式 / 优先路径)
    // -------------------------------------------------------
    if (has_memory_data) {
        std::cout << "正在验证内存中的转换数据..." << std::endl;
        
        for (const auto& [month_key, logs] : context.result.processed_data) {
            auto start_time = std::chrono::steady_clock::now();
            std::set<Error> errors;

            try {
                // [关键逻辑] Core 调用 Serializer: Struct -> JSON
                nlohmann::json json_content = serializer::JsonSerializer::serializeDays(logs);

                // [关键逻辑] Core 调用 Validator: Validate JSON
                // 使用虚拟文件名用于日志显示，表明这是内存数据
                std::string virtual_filename = "MemoryData_" + month_key; 
                
                if (!output_validator.validate_json(virtual_filename, json_content, errors, context.config.date_check_mode)) {
                    printGroupedErrors(virtual_filename, errors);
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
    // -------------------------------------------------------
    // 路径 B: 文件验证 (仅当确实生成了文件时)
    // -------------------------------------------------------
    else if (has_generated_files) {
        std::cout << "正在验证已生成的 JSON 文件..." << std::endl;
        for (const auto& file_to_validate : context.state.generated_files) {
            auto start_time = std::chrono::steady_clock::now();
            std::set<Error> errors;
            
            try {
                std::string content = FileReader::read_content(file_to_validate);
                nlohmann::json json_content = nlohmann::json::parse(content);

                if (!output_validator.validate_json(file_to_validate.string(), json_content, errors, context.config.date_check_mode)) {
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