// converter/validator/source_txt/facade/TextValidator.cpp
#include "TextValidator.hpp"
#include "converter/validator/source_txt/rules/LineRules.hpp"
#include "converter/validator/source_txt/rules/StructureRules.hpp"
#include "common/utils/StringUtils.hpp" 
#include <sstream> 
#include <iostream>

struct TextValidator::PImpl {
    LineRules line_processor;
    StructureRules structural_validator;

    PImpl(const ConverterConfig& config)
        : line_processor(config) {}
};

TextValidator::TextValidator(const ConverterConfig& config)
    : pimpl_(std::make_unique<PImpl>(config)) {}

TextValidator::~TextValidator() = default;

bool TextValidator::validate(const std::string& /*filename*/, const std::string& content, std::set<Error>& errors) {
    // 使用 stringstream 替代 ifstream
    std::stringstream ss(content);
    std::string line;
    int line_number = 0;
    
    while (std::getline(ss, line)) {
        line_number++;
        std::string trimmed_line = trim(line);
        if (trimmed_line.empty()) continue;

        if (pimpl_->line_processor.is_year(trimmed_line)) {
            pimpl_->structural_validator.process_year_line(line_number, trimmed_line, errors);
        } else if (pimpl_->line_processor.is_date(trimmed_line)) {
            pimpl_->structural_validator.process_date_line(line_number, trimmed_line, errors);
        } else if (pimpl_->line_processor.is_remark(trimmed_line)) {
            pimpl_->structural_validator.process_remark_line(line_number, trimmed_line, errors);
        } else if (pimpl_->line_processor.is_valid_event_line(trimmed_line, line_number, errors)) {
            pimpl_->structural_validator.process_event_line(line_number, trimmed_line, errors);
        } else {
            pimpl_->structural_validator.process_unrecognized_line(line_number, trimmed_line, errors);
        }
        
        if (!pimpl_->structural_validator.has_seen_year() && !pimpl_->line_processor.is_year(trimmed_line)) {
             errors.insert({line_number, "The file must start with a year header (e.g., 'y2025').", ErrorType::Source_MissingYearHeader});
             return false;
        }
    }

    return errors.empty();
}