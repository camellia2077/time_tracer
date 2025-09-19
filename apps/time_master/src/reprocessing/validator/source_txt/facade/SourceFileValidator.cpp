// reprocessing/validator/source_txt/facade/SourceFileValidator.cpp
#include "SourceFileValidator.hpp"
// [移除] 不再需要 #include "reprocessing/validator/source_txt/pipelines/SourceValidatorConfig.hpp"
#include "reprocessing/validator/source_txt/pipelines/LineProcessor.hpp"
#include "reprocessing/validator/source_txt/pipelines/StructuralValidator.hpp"
#include "common/utils/StringUtils.hpp" // For trim()

#include <fstream>
#include <iostream>
#include <memory>

// [修改] PImpl 结构体更新
struct SourceFileValidator::PImpl {
    LineProcessor line_processor;
    StructuralValidator structural_validator;

    // [修改] PImpl 的构造函数现在接收 ConverterConfig
    PImpl(const ConverterConfig& config)
        : line_processor(config)
    {}
};

// [修改] 更新构造函数和析构函数
SourceFileValidator::SourceFileValidator(const ConverterConfig& config)
    : pimpl_(std::make_unique<PImpl>(config)) {}

SourceFileValidator::~SourceFileValidator() = default;


bool SourceFileValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream inFile(file_path);
    if (!inFile.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    std::string line;
    int line_number = 0;
    
    while (std::getline(inFile, line)) {
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