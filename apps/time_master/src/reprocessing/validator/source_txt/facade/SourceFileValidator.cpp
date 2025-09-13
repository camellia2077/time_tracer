// reprocessing/validator/source_txt/facade/SourceFileValidator.cpp
#include "SourceFileValidator.hpp"
#include "reprocessing/validator/source_txt/pipelines/SourceValidatorConfig.hpp"
#include "reprocessing/validator/source_txt/pipelines/LineProcessor.hpp"
#include "reprocessing/validator/source_txt/pipelines/StructuralValidator.hpp"

#include <fstream>
#include <iostream>

// PImpl 结构体定义，封装所有内部组件
struct SourceFileValidator::PImpl {
    SourceValidatorConfig config;
    LineProcessor line_processor;
    StructuralValidator structural_validator;

    PImpl(const std::string& config_filename)
        : config(config_filename),
          line_processor(config)
    {}
};

// 构造函数和析构函数
SourceFileValidator::SourceFileValidator(const std::string& config_filename)
    : pimpl_(std::make_unique<PImpl>(config_filename)) {}

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
        
        // 捕捉在循环开始时就出现的结构错误
        if (!pimpl_->structural_validator.has_seen_year() && !pimpl_->line_processor.is_year(trimmed_line)) {
             errors.insert({line_number, "The file must start with a year header (e.g., 'y2025').", ErrorType::Source_MissingYearHeader});
             return false; // 致命错误，提前退出
        }
    }

    return errors.empty();
}