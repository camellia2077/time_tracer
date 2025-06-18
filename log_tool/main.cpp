#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <clocale> // Required for std::setlocale
#include <vector>  // 新增
#include <map>     // 新增

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

// --- 新增辅助函数 ---

// 根据错误类型返回对应的标题字符串
std::string getErrorTypeHeader(FormatValidator::ErrorType type) {
    switch (type) {
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

// 分组并打印错误到控制台和日志文件
void printGroupedErrors(const std::string& filename, const std::set<FormatValidator::Error>& errors, const std::string& error_log_path) {
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;

    // 1. 按错误类型对错误进行分组
    std::map<FormatValidator::ErrorType, std::vector<FormatValidator::Error>> grouped_errors;
    for (const auto& err : errors) {
        grouped_errors[err.type].push_back(err);
    }

    std::ofstream err_stream(error_log_path);
    err_stream << "文件 " << filename << " 的检验错误\n";
    err_stream << "--------------------------------------------------\n\n";

    // 2. 遍历分组后的错误并打印
    for (const auto& pair : grouped_errors) {
        std::string header = getErrorTypeHeader(pair.first);
        
        // 打印到控制台
        std::cerr << "\n" << header << std::endl;
        // 写入日志文件
        err_stream << header << "\n";

        for (const auto& err : pair.second) {
            std::string error_message = "行 " + std::to_string(err.line_number) + ": " + err.message;
            // 打印到控制台
            std::cerr << RED_COLOR << "  " << error_message << RESET_COLOR << std::endl;
            // 写入日志文件
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
    bool validate_only = false;
    std::string input_file;

    if (argc == 2) {
        input_file = argv[1];
    } else if (argc == 3 && std::string(argv[1]) == "-v") {
        validate_only = true;
        input_file = argv[2];
    } else {
        std::cerr << RED_COLOR << "使用方法: " << argv[0] << " <输入文件.txt>" << RESET_COLOR << std::endl;
        std::cerr << RED_COLOR << "或 (仅检验): " << argv[0] << " -v <待检验文件.txt>" << RESET_COLOR << std::endl;
        return 1;
    }

    // --- Configuration File Paths ---
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string error_file = "validation_errors.txt";

    if (validate_only) {
        // --- VALIDATION-ONLY MODE ---
        std::cout << "--- 检验模式: 仅检验文件合法性 ---" << std::endl;

        // Pre-flight check for validator config
        if (!fs::exists(validator_config)) {
            std::cerr << RED_COLOR << "错误: 合法性检验配置文件未找到: " << validator_config << "。程序即将退出。" << RESET_COLOR << std::endl;
            return 1;
        }

        FormatValidator validator(validator_config);
        std::set<FormatValidator::Error> errors;
        bool is_valid = validator.validateFile(input_file, errors);

        std::cout << "\n--- 最终结果 ---" << std::endl;
        if (is_valid) {
            std::cout << GREEN_COLOR << "成功! 文件 '" << input_file << "' 已通过所有合法性检验。" << RESET_COLOR << std::endl;
        } else {
            std::cerr << RED_COLOR << "检验失败。在文件 '" << input_file << "' 中发现错误。" << RESET_COLOR << std::endl;
            // --- 调用新的打印函数 ---
            printGroupedErrors(input_file, errors, error_file);
            return 1;
        }

    } else {
        // --- FULL PROCESS & VALIDATE MODE (DEFAULT) ---
        std::string processed_output_file = "processed_" + fs::path(input_file).filename().string();

        // Pre-flight checks for both config files
        if (!fs::exists(interval_config) || !fs::exists(validator_config)) {
            if(!fs::exists(interval_config)) std::cerr << RED_COLOR << "错误: 转换器配置文件未找到: " << interval_config << "。程序即将退出。" << RESET_COLOR << std::endl;
            if(!fs::exists(validator_config)) std::cerr << RED_COLOR << "错误: 合法性检验配置文件未找到: " << validator_config << "。程序即将退出。" << RESET_COLOR << std::endl;
            return 1;
        }

        // Step 1: Process the initial file
        std::cout << "--- 步骤 1: 正在处理输入文件 ---" << std::endl;
        IntervalProcessor processor(interval_config);
        if (!processor.processFile(input_file, processed_output_file)) {
            std::cerr << RED_COLOR << "处理输入文件失败。程序即将中止。" << RESET_COLOR << std::endl;
            return 1;
        }
        std::cout << GREEN_COLOR << "初始处理完成。输出已写入: " << processed_output_file << RESET_COLOR << std::endl;
        std::cout << "---------------------------------------" << std::endl;

        // Step 2: Validate the processed file
        std::cout << "\n--- 步骤 2: 正在检验已处理的文件 ---" << std::endl;
        FormatValidator validator(validator_config);
        std::set<FormatValidator::Error> errors;
        bool is_valid = validator.validateFile(processed_output_file, errors);

        // Step 3: Report the final result
        std::cout << "\n--- 最终结果 ---" << std::endl;
        if (is_valid) {
            std::cout << GREEN_COLOR << "成功! 生成的文件 '" << processed_output_file << "' 已通过所有合法性检验。" << RESET_COLOR << std::endl;
        } else {
            std::cerr << RED_COLOR << "检验失败。在生成的文件 '" << processed_output_file << "' 中发现错误。" << RESET_COLOR << std::endl;
            // --- 调用新的打印函数 ---
            printGroupedErrors(processed_output_file, errors, error_file);
            return 1;
        }
    }

    return 0; // Success
}