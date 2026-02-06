// application/pipeline/steps/logic_validator_step.cpp
#include "application/pipeline/steps/logic_validator_step.hpp"

#include <iostream>

#include "common/ansi_colors.hpp"
#include "serializer/json_serializer.hpp"  // 需要序列化后进行 JSON 验证
#include "validator/json/facade/json_validator.hpp"

namespace core::pipeline {

auto LogicValidatorStep::Execute(PipelineContext& context) -> bool {

  std::cout << "Step: Validating Business Logic (Dates, Continuity)..."
            << std::endl;

  if (context.result.processed_data.empty()) {
    std::cout << time_tracer::common::colors::kYellow << "No data to validate." << time_tracer::common::colors::kReset
              << std::endl;
    return true;
  }

  // 初始化 JSON 验证器 (注入日期检查模式)
  validator::json::JsonValidator validator(context.config.date_check_mode);

  bool all_valid = true;

  // 遍历内存中的数据 (按月分组)
  for (const auto& [month_key, days] : context.result.processed_data) {
    if (days.empty()) {
      continue;
    }

    // 为了复用现有的 JsonValidator 逻辑，我们将 Struct 临时序列化为 JSON
    // 这样避免了重写一套针对 Struct 的验证器，且逻辑保持一致
    nlohmann::json json_content =
        serializer::JsonSerializer::SerializeDays(days);

    // 构造一个虚拟文件名用于报错显示
    std::string pseudo_filename = "ProcessedData[" + month_key + "]";

    std::set<validator::Error> errors;
    if (!validator.Validate(pseudo_filename, json_content, errors)) {

      all_valid = false;
      validator::PrintGroupedErrors(pseudo_filename, errors);
    }
  }

  if (all_valid) {
    std::cout << time_tracer::common::colors::kGreen << "Logic validation passed." << time_tracer::common::colors::kReset
              << std::endl;
  } else {
    std::cerr << time_tracer::common::colors::kRed
              << "Logic validation found issues (e.g., broken date continuity)."
              << time_tracer::common::colors::kReset << std::endl;
  }

  return all_valid;
}

}  // namespace core::pipeline