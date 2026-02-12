// domain/logic/validator/txt/facade/text_validator.cpp
#include "domain/logic/validator/txt/facade/text_validator.hpp"

#include <iostream>
#include <sstream>

#include "domain/logic/validator/txt/rules/line_rules.hpp"
#include "domain/logic/validator/txt/rules/structure_rules.hpp"
#include "shared/utils/string_utils.hpp"

namespace validator::txt {

struct TextValidator::PImpl {
  LineRules line_processor;
  StructureRules structural_validator;

  PImpl(const ConverterConfig& config) : line_processor(config) {}
};

TextValidator::TextValidator(const ConverterConfig& config)
    : pimpl_(std::make_unique<PImpl>(config)) {}

TextValidator::~TextValidator() = default;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto TextValidator::Validate(const std::string& filename,
                             const std::string& content,
                             std::set<Error>& errors) -> bool {
  // [关键修复] 每次验证新文件前，必须重置结构验证器的状态
  // 否则上一个文件的状态（如 has_seen_year）会影响当前文件，导致"Multiple year
  // headers"误报
  pimpl_->structural_validator.Reset();

  std::stringstream content_ss(content);
  std::string line;
  int line_number = 0;

  while (std::getline(content_ss, line)) {
    line_number++;
    std::string trimmed_line = Trim(line);
    if (trimmed_line.empty()) {
      continue;
    }
    SourceSpan span{.file_path = filename,
                    .line_start = line_number,
                    .line_end = line_number,
                    .column_start = 1,
                    .column_end = static_cast<int>(line.length()),
                    .raw_text = line};

    if (LineRules::IsYear(trimmed_line)) {
      pimpl_->structural_validator.ProcessYearLine(line_number, trimmed_line,
                                                   errors, span);
    } else if (LineRules::IsDate(trimmed_line)) {
      pimpl_->structural_validator.ProcessDateLine(line_number, trimmed_line,
                                                   errors, span);
    } else if (pimpl_->line_processor.IsRemark(trimmed_line)) {
      pimpl_->structural_validator.ProcessRemarkLine(line_number, trimmed_line,
                                                     errors, span);
    } else if (pimpl_->line_processor.IsValidEventLine(
                   trimmed_line, line_number, errors, span)) {
      pimpl_->structural_validator.ProcessEventLine(line_number, trimmed_line,
                                                    errors, span);
    } else {
      StructureRules::ProcessUnrecognizedLine(line_number, trimmed_line, errors,
                                              span);
    }

    // 检查文件头是否缺失年份
    // 注意：这里逻辑稍微调整，只有当遇到有效内容（且还没见年份）时才报错，或者文件结束检查
    // 原有逻辑在每一行都查，可能会报多次，但这里保持原样以确保尽早发现
    if (!pimpl_->structural_validator.HasSeenYear() &&
        !LineRules::IsYear(trimmed_line)) {
      errors.insert({line_number,
                     "The file must start with a year header (e.g., 'y2025').",
                     ErrorType::kSourceMissingYearHeader, span});
      return false;  // 严重错误，停止解析
    }
  }

  return errors.empty();
}

}  // namespace validator::txt
