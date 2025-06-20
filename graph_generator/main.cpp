// main.cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <clocale> // Required for std::setlocale
#include <vector>
#include <map>
#include <algorithm> // for std::sort

#include "IntervalProcessor.h"
#include "FormatValidator.h"
#include "SharedUtils.h"

// For platform-specific UTF-8 console setup
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

// Sets up the console to correctly display UTF-8 characters.
void setup_console_for_utf8() {
#ifdef _WIN32
    // Set console code page to UTF-8 on Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    // For Linux/macOS, set the locale to the system's default.
    try {
        std::setlocale(LC_ALL, "");
    } catch (...) {
        std::cerr << YELLOW_COLOR << "Warning: Could not set locale. UTF-8 display might be affected." << RESET_COLOR << std::endl;
    }
#endif
}

// --- START CORRECTION ---
// The function is updated to group date-related errors under a new header.
std::string getErrorTypeHeader(FormatValidator::ErrorType type) {
    switch (type) {
        // New cases to handle specific date errors
        case FormatValidator::ErrorType::DateContinuity:
        case FormatValidator::ErrorType::IncorrectDayCountForMonth:
            return "Date errors(日期错误):";

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
// --- END CORRECTION ---

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
        std::string header = getErrorTypeHeader(pair.first); // This will now use the new grouping
        std::cerr << "\n" << header << std::endl;
        err_stream << header << "\n";
        for (const auto& err : pair.second) {
            std::string error_message = "Line " + std::to_string(err.line_number) + ": " + err.message;
            std::cerr << RED_COLOR << "  " << error_message << RESET_COLOR << std::endl;
            err_stream << "  " << error_message << "\n";
        }
    }
    err_stream.close();
    std::cout << "\n详细的错误日志已保存至: " << YELLOW_COLOR << error_log_path << RESET_COLOR << std::endl;
}


int main(int argc, char* argv[]) {
    // --- Setup ---
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // --- Argument Parsing ---
    bool process = false;
    bool validate = false;
    std::string input_path_str;

    if (argc != 3) {
        std::cerr << RED_COLOR << "使用方法: " << argv[0] << " <flag> <文件或文件夹路径>" << RESET_COLOR << std::endl;
        std::cerr << "  flags:" << std::endl;
        std::cerr << "    -p\t只读取后转换文件,不检验内容合法性" << std::endl;
        std::cerr << "    -pv\t读取文件转换,并且检验合法性" << std::endl;
        std::cerr << "    -v\t只检验,不转换文件" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    input_path_str = argv[2];

    if (flag == "-p"|| flag == "-P") {
        process = true;
    } else if (flag == "-pv"|| flag == "-PV") {
        process = true;
        validate = true;
    } else if (flag == "-v"||flag == "-V") {
        validate = true;
    } else {
        std::cerr << RED_COLOR << "Errors: " << RESET_COLOR <<  "未知的 flag '" << flag << "'" << std::endl;
        std::cerr << "使用方法: " << argv[0] << " <flag> <文件或文件夹路径>" << RESET_COLOR << std::endl;
        std::cerr << "  flags:" << std::endl;
        std::cerr << "    -p\t只读取后转换文件,不检验内容合法性" << std::endl;
        std::cerr << "    -pv\t读取文件转换,并且检验合法性" << std::endl;
        std::cerr << "    -v\t只检验,不转换文件" << std::endl;
        return 1;
    }


    // --- Config File Paths ---
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string header_config = "header_format.json";
    std::string error_file = "validation_errors.txt";

    // Clear previous error log
    std::ofstream ofs(error_file, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    // --- File/Directory Path Handling ---
    fs::path input_path(input_path_str);
    std::vector<fs::path> files_to_process;

    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "Errors: "<< RESET_COLOR << "输入的路径不存在: " << input_path_str  << std::endl;
        return 1;
    }

    if (fs::is_directory(input_path)) {
        std::cout << "检测到输入为文件夹,将处理其中所有的 .txt 文件..." << std::endl;
        for (const auto& entry : fs::directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                files_to_process.push_back(entry.path());
            }
        }
        if (files_to_process.empty()) {
            std::cout << YELLOW_COLOR << "Warring: " << RESET_COLOR<<  "在文件夹 " << input_path_str << " 中未找到 .txt 文件。"  << std::endl;
            return 0;
        }
        std::sort(files_to_process.begin(), files_to_process.end());
    } else if (fs::is_regular_file(input_path)) {
        files_to_process.push_back(input_path);
    } else {
        std::cerr << RED_COLOR << "Error: " <<  RESET_COLOR <<"输入的路径既不是文件也不是文件夹: " << input_path_str << std::endl;
        return 1;
    }

    // --- Initialize Counters ---
    int success_count = 0;
    int failure_count = 0;
    int conversion_success_count = 0;
    int conversion_failure_count = 0;

    // --- Loop to process all found files ---
    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n"; // Start of wrapper
        std::cout << "正在处理文件: " << file.string() << "\n";
        
        std::string file_to_validate = file.string();
        bool processing_successful = true;

        // Step 1: Conversion (if required)
        if (process) {
            std::string processed_output_file = "processed_" + file.filename().string();
            IntervalProcessor processor(interval_config, header_config);
            if (!processor.processFile(file.string(), processed_output_file)) {
                std::cerr << RED_COLOR << "Errors: " << RESET_COLOR << "处理文件失败。跳过此文件。" << std::endl;
                processing_successful = false;
                conversion_failure_count++;
            } else {
                std::cout << GREEN_COLOR << "Succeeded: " << RESET_COLOR << "File conversion complete. Output written to: " << processed_output_file << std::endl;
                file_to_validate = processed_output_file;
                conversion_success_count++;
            }
        }

        if (!processing_successful) {
            std::cout << "=======================================================\n"; // End of wrapper for failed processing
            continue; // Skip to next file
        }

        // Step 2: Validation (if required)
        if (validate) {
            FormatValidator validator(validator_config, header_config);
            std::set<FormatValidator::Error> errors;
            bool is_valid = validator.validateFile(file_to_validate, errors);

            if (is_valid) {
                std::cout << GREEN_COLOR << "\nSuccess: "<< RESET_COLOR << "This file has passed all validity checks." << std::endl;
                success_count++;
            } else {
                std::cerr << RED_COLOR << "\nErrors: " << RESET_COLOR << "Mistakes were found in the file."  << std::endl;
                printGroupedErrors(file_to_validate, errors, error_file);
                failure_count++;
            }
        }

        std::cout << "=======================================================\n"; // End of wrapper
    }

    // --- Final Output ---
    std::cout << "\n--- 所有任务处理完毕 ---" << std::endl;
    std::cout << "格式转换成功的txt数量:" << conversion_success_count << std::endl;
    std::cout << "格式转换失败的txt数量:" << conversion_failure_count << std::endl;
    std::cout  << std::endl;
    std::cout << "检验成功的txt数量:" << success_count << std::endl;
    std::cout << "检验失败的txt数量:" << failure_count << std::endl;

    return 0;
}