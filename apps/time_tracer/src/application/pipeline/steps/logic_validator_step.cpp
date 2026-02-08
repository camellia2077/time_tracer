// application/pipeline/steps/logic_validator_step.cpp
#include "application/pipeline/steps/logic_validator_step.hpp"

#include <iostream>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/logic/validator/structure/facade/struct_validator.hpp"
#include "shared/types/ansi_colors.hpp"

namespace core::pipeline {

auto LogicValidatorStep::Execute(PipelineContext& context) -> bool {
  std::cout << "Step: Validating Business Logic (Dates, Continuity)..."
            << std::endl;

  if (context.result.processed_data.empty()) {
    std::cout << time_tracer::common::colors::kYellow << "No data to validate."
              << time_tracer::common::colors::kReset << std::endl;
    return true;
  }

  // 初始化 Struct 验证器
  // [Performance Optimization]
  // Previous implementation serialized structs to JSON to reuse JSON
  // validators, which caused significant overhead. We now use native C++
  // structs (DailyLog) for validation to avoid serialization costs and ensure
  // type safety.
  validator::structure::StructValidator validator(
      context.config.date_check_mode);

  bool all_valid = true;

  // 遍历内存中的数据 (按月分组)
  for (const auto& [month_key, days] : context.result.processed_data) {
    if (days.empty()) {
      continue;
    }

    // 构造一个虚拟文件名用于报错显示
    std::string pseudo_filename = "ProcessedData[" + month_key + "]";

    std::vector<validator::Diagnostic> diagnostics;
    // 直接验证 Struct 数据
    if (!validator.Validate(pseudo_filename, days, diagnostics)) {
      all_valid = false;
      validator::PrintDiagnostics(pseudo_filename, diagnostics);
    }
  }

  if (all_valid) {
    std::cout << time_tracer::common::colors::kGreen
              << "Logic validation passed."
              << time_tracer::common::colors::kReset << std::endl;
  } else {
    std::cerr << time_tracer::common::colors::kRed
              << "Logic validation found issues (e.g., broken date continuity)."
              << time_tracer::common::colors::kReset << std::endl;
  }

  return all_valid;
}

}  // namespace core::pipeline
