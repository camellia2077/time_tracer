// time_master/reprocessing/validator/ValidatorUtils.cpp
#include "ValidatorUtils.hpp"
#include "common/AnsiColors.hpp" // For colored console output
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// 根据错误类型获取分组标题
static std::string getErrorTypeHeader(ErrorType type) {
    switch (type) {
        case ErrorType::Source_RemarkAfterEvent:
        case ErrorType::Source_NoDateAtStart:
        case ErrorType::Source_InvalidLineFormat:
        case ErrorType::UnrecognizedActivity:
        case ErrorType::Source_MissingYearHeader: // [新增]
            return "Source file format errors (源文件格式错误):";
        case ErrorType::IncorrectDayCountForMonth:
            return "Date errors(日期错误):";
        case ErrorType::DateContinuity:
        case ErrorType::TimeDiscontinuity:
            return "Time discontinuity errors(时间不连续):";
        case ErrorType::MissingSleepNight:
            return "Lack of sleep_night errors(最后的活动项目缺少sleep_night):";
        case ErrorType::FileAccess:
            return "File access errors:";
        case ErrorType::Structural:
            return "Structural errors:";
        case ErrorType::LineFormat:
            return "Line format errors:";
        case ErrorType::Logical:
            return "Logical consistency errors:";
        default:
            return "Other errors:";
    }
}

// ... (printGroupedErrors 和 trim 函数保持不变) ...
void printGroupedErrors(const std::string& filename, const std::set<Error>& errors, const std::string& error_log_path) {
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
    std::map<ErrorType, std::vector<Error>> grouped_errors;
    for (const auto& err : errors) {
        grouped_errors[err.type].push_back(err);
    }

    std::ofstream err_stream(error_log_path, std::ios::app);
    err_stream << "\n文件 " << filename << " 的检验错误\n--------------------------------------------------\n\n";

    for (const auto& pair : grouped_errors) {
        std::string header = getErrorTypeHeader(pair.first);
        std::cerr << "\n" << header << std::endl;
        err_stream << header << "\n";
        for (const auto& err : pair.second) {
            std::string error_message = (err.line_number == 0) ? err.message : "Line " + std::to_string(err.line_number) + ": " + err.message;
            std::cerr << "  " << error_message << std::endl;
            err_stream << "  " << error_message << "\n";
        }
    }
    err_stream.close();
    std::cout << "\n详细的错误日志已保存至: " << YELLOW_COLOR << error_log_path << RESET_COLOR << std::endl;
}

std::string trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}