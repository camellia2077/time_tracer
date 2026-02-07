// application/pipeline/steps/structure_validator_step.cpp
#include "application/pipeline/steps/structure_validator_step.hpp"

#include <iostream>

#include "domain/logic/validator/txt/facade/text_validator.hpp"
#include "infrastructure/io/file_import_reader.hpp"
#include "shared/types/ansi_colors.hpp"

namespace core::pipeline {

auto StructureValidatorStep::Execute(PipelineContext& context) -> bool {
  std::cout << "Step: Validating Source Structure (TXT)..." << std::endl;

  // 使用上下文中的配置初始化验证器
  validator::txt::TextValidator validator(context.state.converter_config);

  bool all_valid = true;
  int files_checked = 0;

  for (const auto& file_path : context.state.source_files) {
    files_checked++;
    std::string filename = file_path.filename().string();

    // 读取文件内容
    std::string content = FileReader::ReadContent(file_path);

    std::set<validator::Error> errors;

    // 执行验证
    if (!validator.Validate(filename, content, errors)) {
      all_valid = false;
      // 打印错误 (复用 ValidatorUtils)
      validator::PrintGroupedErrors(filename, errors);
    }
  }

  if (all_valid) {
    std::cout << time_tracer::common::colors::kGreen
              << "Structure validation passed for " << files_checked
              << " files." << time_tracer::common::colors::kReset << std::endl;
  } else {
    std::cerr << time_tracer::common::colors::kRed
              << "Structure validation failed. Please fix the errors above."
              << time_tracer::common::colors::kReset << std::endl;
  }

  return all_valid;
}

}  // namespace core::pipeline
