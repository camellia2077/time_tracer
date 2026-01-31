// application/pipeline/steps/structure_validator_step.cpp
#include "structure_validator_step.hpp"

#include <iostream>

#include "common/ansi_colors.hpp"
#include "infrastructure/io/file_import_reader.hpp"
#include "validator/txt/facade/TextValidator.hpp"

namespace core::pipeline {

auto StructureValidatorStep::execute(PipelineContext& context) -> bool {
  std::cout << "Step: Validating Source Structure (TXT)..." << std::endl;

  // 使用上下文中的配置初始化验证器
  validator::txt::TextValidator validator(context.state.converter_config);

  bool all_valid = true;
  int files_checked = 0;

  for (const auto& file_path : context.state.source_files) {
    files_checked++;
    std::string filename = file_path.filename().string();

    // 读取文件内容
    std::string content = FileReader::read_content(file_path);

    std::set<validator::Error> errors;

    // 执行验证
    if (!validator.validate(filename, content, errors)) {
      all_valid = false;
      // 打印错误 (复用 ValidatorUtils)
      validator::printGroupedErrors(filename, errors);
    }
  }

  if (all_valid) {
    std::cout << GREEN_COLOR << "Structure validation passed for "
              << files_checked << " files." << RESET_COLOR << std::endl;
  } else {
    std::cerr << RED_COLOR
              << "Structure validation failed. Please fix the errors above."
              << RESET_COLOR << std::endl;
  }

  return all_valid;
}

}  // namespace core::pipeline
