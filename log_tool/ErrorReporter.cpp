// ErrorReporter.cpp

#include "ErrorReporter.h"
#include "SharedUtils.h" // For color codes

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

namespace { // Anonymous namespace for internal linkage (like static)

// This helper function is now private to this .cpp file.
std::string getErrorTypeHeader(FormatValidator::ErrorType type) {
    switch (type) {
        // MODIFICATION: DateContinuity is no longer here.
        case FormatValidator::ErrorType::IncorrectDayCountForMonth:
            return "Date errors(日期错误):";

        // MODIFICATION: DateContinuity is now grouped with TimeDiscontinuity.
        case FormatValidator::ErrorType::DateContinuity:
        case FormatValidator::ErrorType::TimeDiscontinuity:
            return "Time discontinuity errors(时间不连续):";

        case FormatValidator::ErrorType::MissingSleepNight:
            return "Lack of sleep_night errors(最后的活动项目缺少sleep_night):";
        case FormatValidator::ErrorType::FileAccess:
            return "File access errors:";
        case FormatValidator::ErrorType::Structural:
            return "Structural errors:";
        case FormatValidator::ErrorType::LineFormat:
            return "Line format errors:";
        case FormatValidator::ErrorType::Logical:
            return "Logical consistency errors:";
        default:
            return "Other errors:";
    }
}

} // namespace

namespace ErrorReporter {

void printGroupedErrors(const std::string& filename, const std::set<FormatValidator::Error>& errors, const std::string& error_log_path) {
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
    std::map<FormatValidator::ErrorType, std::vector<FormatValidator::Error>> grouped_errors;
    for (const auto& err : errors) {
        grouped_errors[err.type].push_back(err);
    }
    
    std::ofstream err_stream(error_log_path, std::ios::app); // Use append mode
    err_stream << "\n文件 " << filename << " 的检验错误\n";
    err_stream << "--------------------------------------------------\n\n";
    
    for (const auto& pair : grouped_errors) {
        std::string header = getErrorTypeHeader(pair.first);
        std::cerr << "\n" << header << std::endl;
        err_stream << header << "\n";
        for (const auto& err : pair.second) {
            // --- MODIFICATION START ---
            // Conditionally format the error message based on the line number.
            std::string error_message;
            if (err.line_number == 0) {
                // For file-level errors (like month day count), don't print the line number.
                error_message = err.message;
            } else {
                // For all other errors, include the line number.
                error_message = "Line " + std::to_string(err.line_number) + ": " + err.message;
            }
            // --- MODIFICATION END ---
            
            std::cerr << RED_COLOR << "  " << error_message << RESET_COLOR << std::endl;
            err_stream << "  " << error_message << "\n";
        }
    }
    
    err_stream.close();
    std::cout << "\n详细的错误日志已保存至: " << YELLOW_COLOR << error_log_path << RESET_COLOR << std::endl;
}

} // namespace ErrorReporter