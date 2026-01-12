// converter/LogProcessor.cpp
#include "converter/LogProcessor.hpp"
#include "converter/validator/FileValidator.hpp"
#include "converter/convert/facade/ConverterService.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>

LogProcessor::LogProcessor(const ConverterConfig& config) : config_(config) {}

void LogProcessor::convertStreamToData(std::istream& combined_stream, std::function<void(DailyLog&&)> data_consumer) {
    try {
        ConverterService processor(config_);
        processor.executeConversion(combined_stream, data_consumer);
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "An error occurred during conversion: " << e.what() << RESET_COLOR << std::endl;
    }
}

ProcessingResult LogProcessor::processSourceContent(const std::string& filename, const std::string& content) {
    ProcessingResult result;
    result.success = true;
    
    FileValidator validator(config_);
    std::set<Error> errors;
    
    if (!validator.validate_source(filename, content, errors)) {
        printGroupedErrors(filename, errors);
        result.success = false;
    }
    
    return result;
}