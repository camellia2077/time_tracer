// validator/common/ValidatorUtils.cpp
#include "ValidatorUtils.hpp"
#include "common/AnsiColors.hpp" 
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

namespace validator {

static std::string getErrorTypeHeader(ErrorType type) {
    switch (type) {
        case ErrorType::Source_RemarkAfterEvent:
        case ErrorType::Source_NoDateAtStart:
        case ErrorType::Source_InvalidLineFormat:
        case ErrorType::UnrecognizedActivity:
        case ErrorType::Source_MissingYearHeader:
            return "Source file format errors (源文件格式错误):";
        case ErrorType::IncorrectDayCountForMonth:
            return "Date Logic errors(日期逻辑错误):";
        case ErrorType::DateContinuity:
            return "Date Continuity errors (日期中断/缺失):";
        case ErrorType::TimeDiscontinuity:
            return "Activity Time Discontinuity errors (活动时间不连续):";
        case ErrorType::MissingSleepNight:
            return "Lack of sleep activity errors(最后的活动项目缺少sleep活动):";
        case ErrorType::Json_TooFewActivities:
            return "Activity count errors(活动数量错误):";
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

void printGroupedErrors(const std::string& filename, const std::set<Error>& errors) {
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
    std::map<ErrorType, std::vector<Error>> grouped_errors;
    for (const auto& err : errors) {
        grouped_errors[err.type].push_back(err);
    }

    const std::string error_log_path = "./output/errors.log"; // 建议后续改为可配置路径
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

} // namespace validator