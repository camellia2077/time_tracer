#include "logic_validator_step.hpp"
#include "validator/json/facade/JsonValidator.hpp"
#include "serializer/json_serializer.hpp" // 需要序列化后进行 JSON 验证
#include "common/ansi_colors.hpp"
#include <iostream>

namespace core::pipeline {

bool LogicValidatorStep::execute(PipelineContext& context) {
    std::cout << "Step: Validating Business Logic (Dates, Continuity)..." << std::endl;

    if (context.result.processed_data.empty()) {
        std::cout << YELLOW_COLOR << "No data to validate." << RESET_COLOR << std::endl;
        return true;
    }

    // 初始化 JSON 验证器 (注入日期检查模式)
    validator::json::JsonValidator validator(context.config.date_check_mode);
    
    bool all_valid = true;

    // 遍历内存中的数据 (按月分组)
    for (const auto& [month_key, days] : context.result.processed_data) {
        if (days.empty()) continue;

        // 为了复用现有的 JsonValidator 逻辑，我们将 Struct 临时序列化为 JSON
        // 这样避免了重写一套针对 Struct 的验证器，且逻辑保持一致
        nlohmann::json json_content = serializer::JsonSerializer::serializeDays(days);
        
        // 构造一个虚拟文件名用于报错显示
        std::string pseudo_filename = "ProcessedData[" + month_key + "]";

        std::set<validator::Error> errors;
        if (!validator.validate(pseudo_filename, json_content, errors)) {
            all_valid = false;
            validator::printGroupedErrors(pseudo_filename, errors);
        }
    }

    if (all_valid) {
        std::cout << GREEN_COLOR << "Logic validation passed." << RESET_COLOR << std::endl;
    } else {
        std::cerr << RED_COLOR << "Logic validation found issues (e.g., broken date continuity)." << RESET_COLOR << std::endl;
    }

    return all_valid;
}

} // namespace core::pipeline