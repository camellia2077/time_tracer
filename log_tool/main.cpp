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

// --- 错误处理函数 (不变) ---

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

void printGroupedErrors(const std::string& filename, const std::set<FormatValidator::Error>& errors, const std::string& error_log_path) {
    std::cerr << "请根据以下错误信息，手动修正该文件。" << std::endl;
    std::map<FormatValidator::ErrorType, std::vector<FormatValidator::Error>> grouped_errors;
    for (const auto& err : errors) {
        grouped_errors[err.type].push_back(err);
    }
    std::ofstream err_stream(error_log_path, std::ios::app); // 使用追加模式
    err_stream << "\n文件 " << filename << " 的检验错误\n";
    err_stream << "--------------------------------------------------\n\n";
    for (const auto& pair : grouped_errors) {
        std::string header = getErrorTypeHeader(pair.first);
        std::cerr << "\n" << header << std::endl;
        err_stream << header << "\n";
        for (const auto& err : pair.second) {
            std::string error_message = "行 " + std::to_string(err.line_number) + ": " + err.message;
            std::cerr << RED_COLOR << "  " << error_message << RESET_COLOR << std::endl;
            err_stream << "  " << error_message << "\n";
        }
    }
    err_stream.close();
    std::cout << "\n详细的错误日志已保存至: " << YELLOW_COLOR << error_log_path << RESET_COLOR << std::endl;
}

// --- 核心处理函数，用于处理单个文件 ---
void processSingleFile(const fs::path& filePath, bool validate_only, const std::string& interval_config, const std::string& validator_config, const std::string& header_config, const std::string& error_file) {
    std::cout << "\n=======================================================\n";
    std::cout << "正在处理文件: " << filePath.string() << "\n";
    std::cout << "=======================================================\n";

    if (validate_only) {
        // --- 检验模式 ---
        FormatValidator validator(validator_config, header_config);
        std::set<FormatValidator::Error> errors;
        bool is_valid = validator.validateFile(filePath.string(), errors);

        if (is_valid) {
            std::cout << GREEN_COLOR << "成功! 文件已通过所有合法性检验。" << RESET_COLOR << std::endl;
        } else {
            std::cerr << RED_COLOR << "检验失败。在文件中发现错误。" << RESET_COLOR << std::endl;
            printGroupedErrors(filePath.string(), errors, error_file);
        }
    } else {
        // --- 完整处理模式 ---
        std::string processed_output_file = "processed_" + filePath.filename().string();
        
        // 步骤 1: 处理文件
        IntervalProcessor processor(interval_config, header_config);
        if (!processor.processFile(filePath.string(), processed_output_file)) {
            std::cerr << RED_COLOR << "处理文件失败。跳过此文件。" << RESET_COLOR << std::endl;
            return;
        }
        std::cout << "初始处理完成。输出已写入: " << processed_output_file << std::endl;
        
        // 步骤 2: 检验已处理的文件
        FormatValidator validator(validator_config, header_config);
        std::set<FormatValidator::Error> errors;
        bool is_valid = validator.validateFile(processed_output_file, errors);

        if (is_valid) {
            std::cout << GREEN_COLOR << "成功! 生成的文件已通过所有合法性检验。" << RESET_COLOR << std::endl;
        } else {
            std::cerr << RED_COLOR << "检验失败。在生成的文件中发现错误。" << RESET_COLOR << std::endl;
            printGroupedErrors(processed_output_file, errors, error_file);
        }
    }
}


int main(int argc, char* argv[]) {
    // --- 设置 ---
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // ... 参数解析部分不变 ...
    bool validate_only = false;
    std::string input_path_str;

    if (argc == 2) {
        input_path_str = argv[1];
    } else if (argc == 3 && std::string(argv[1]) == "-v") {
        validate_only = true;
        input_path_str = argv[2];
    } else {
        std::cerr << RED_COLOR << "使用方法 (处理单个文件): " << argv[0] << " <文件路径.txt>" << RESET_COLOR << std::endl;
        std::cerr << RED_COLOR << "使用方法 (处理文件夹): " << argv[0] << " <文件夹路径>" << RESET_COLOR << std::endl;
        std::cerr << RED_COLOR << "或 (仅检验): " << argv[0] << " -v <文件或文件夹路径>" << RESET_COLOR << std::endl;
        return 1;
    }

    // --- 配置文件路径 ---
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string header_config = "header_format.json"; // 新增
    std::string error_file = "validation_errors.txt";

    // 清空之前的错误日志
    std::ofstream ofs(error_file, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    // ... 文件/文件夹路径处理部分不变 ...
    fs::path input_path(input_path_str);
    std::vector<fs::path> files_to_process;

    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path_str << RESET_COLOR << std::endl;
        return 1;
    }

    if (fs::is_directory(input_path)) {
        std::cout << "检测到输入为文件夹，将处理其中所有的 .txt 文件..." << std::endl;
        for (const auto& entry : fs::directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                files_to_process.push_back(entry.path());
            }
        }
        if (files_to_process.empty()) {
            std::cout << YELLOW_COLOR << "警告: 在文件夹 " << input_path_str << " 中未找到 .txt 文件。" << RESET_COLOR << std::endl;
            return 0;
        }
        std::sort(files_to_process.begin(), files_to_process.end());
    } else if (fs::is_regular_file(input_path)) {
        files_to_process.push_back(input_path);
    } else {
        std::cerr << RED_COLOR << "错误: 输入的路径既不是文件也不是文件夹: " << input_path_str << RESET_COLOR << std::endl;
        return 1;
    }

    // --- 循环处理所有找到的文件 ---
    // 修改：将 header_config 传递给处理函数
    for (const auto& file : files_to_process) {
        processSingleFile(file, validate_only, interval_config, validator_config, header_config, error_file);
    }

    std::cout << "\n--- 所有任务处理完毕 ---" << std::endl;

    return 0;
}