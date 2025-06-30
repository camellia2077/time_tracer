// main.cpp (已支持 -a 命令和错误即停机制)
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <clocale>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "IntervalProcessor.h"
#include "FormatValidator.h"
#include "SharedUtils.h"
#include "ErrorReporter.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

void setup_console_for_utf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    try {
        std::setlocale(LC_ALL, "");
    } catch (...) {
        std::cerr << YELLOW_COLOR << "Warning: Could not set locale. UTF-8 display might be affected." << RESET_COLOR << std::endl;
    }
#endif
}

// 函数：打印使用说明
void print_usage(const char* app_name) {
    std::cerr << GREEN_COLOR << "使用方法: " << app_name << " <路径> [标志...]" << RESET_COLOR << std::endl;
    std::cerr << "\n必须提供 <文件或文件夹路径>." << std::endl;
    std::cerr << "\n主要动作标志:" << std::endl;
    std::cerr << "  -a,  --all\t\t\t执行完整流程(检验源->转换->检验输出)，任何错误都会立即终止程序。" << std::endl;
    std::cerr << "  -c,  --convert\t\t仅转换文件格式。" << std::endl;
    std::cerr << "  -vs, --validate-source\t仅检验源文件的格式。" << std::endl;
    
    std::cerr << "\n可选的附加标志:" << std::endl;
    std::cerr << "  -vo, --validate-output\t转换后检验输出文件 (必须与 -c 或 -a 一起使用)。" << std::endl;
    std::cerr << "  -edc, --enable-day-check\t启用对月份天数完整性的检查。" << std::endl;
    
    std::cerr << "\n其他:" << std::endl;
    std::cerr << "  -h,  --help\t\t\t显示此帮助信息。" << std::endl;
    std::cerr << "  -v,  --version\t\t显示版本号和更新日期。" << std::endl;
}


int main(int argc, char* argv[]) {
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    const std::string APP_VERSION = "0.4.0"; // Version updated for --all command
    const std::string LAST_UPDATE_DATE = "2025-06-30"; 
    
    // --- 参数解析逻辑 ---
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
    bool run_all = false; // 新增: -a 模式的标志
    std::string input_path_str;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::vector<std::string> args(argv + 1, argv + argc);

    for (const auto& arg : args) {
        if (arg == "-h" || arg == "--help") { print_usage(argv[0]); return 0; }
        if (arg == "-v" || arg == "--version") { std::cout << "版本号: " << APP_VERSION << "\n最后更新时间: " << LAST_UPDATE_DATE << std::endl; return 0; }
    }
    
    for (const auto& arg : args) {
        if (arg == "-a" || arg == "--all") run_all = true;
        else if (arg == "-c" || arg == "--convert") convert = true;
        else if (arg == "-vs" || arg == "--validate-source") validate_source = true;
        else if (arg == "-vo" || arg == "--validate-output") validate_output = true;
        else if (arg == "-edc" || arg == "--enable-day-check") enable_day_count_check = true;
        else if (arg.rfind("-", 0) != 0) {
            if (!input_path_str.empty()) { std::cerr << RED_COLOR << "错误: 检测到多个输入路径。请只提供一个。" << RESET_COLOR << std::endl; return 1; }
            input_path_str = arg;
        } else {
            if (arg != "-h" && arg != "--help" && arg != "-v" && arg != "--version") { std::cerr << YELLOW_COLOR << "警告: 未知的标志 '" << arg << "'" << RESET_COLOR << std::endl; }
        }
    }
    
    if (run_all) {
        validate_source = true;
        convert = true;
        validate_output = true;
    }

    if (input_path_str.empty()) { std::cerr << RED_COLOR << "错误: 未提供输入文件或文件夹路径。" << RESET_COLOR << std::endl; print_usage(argv[0]); return 1; }
    if (!convert && !validate_source) { std::cerr << RED_COLOR << "错误: 必须指定至少一个主要动作 (-c, -vs, 或 -a)。" << RESET_COLOR << std::endl; print_usage(argv[0]); return 1; }
    if (validate_output && !convert) { std::cerr << RED_COLOR << "错误: -vo 必须与 -c 或 -a 一起使用。" << RESET_COLOR << std::endl; return 1; }
    
    auto total_start_time = std::chrono::high_resolution_clock::now();
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string error_file = "validation_errors.txt";
    
    fs::path input_path(input_path_str);
    std::vector<fs::path> files_to_process;

    if (!fs::exists(input_path)) { std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path_str  << std::endl; return 1; }

    bool is_input_a_directory = fs::is_directory(input_path);
    if (is_input_a_directory) {
        for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") files_to_process.push_back(entry.path());
        }
        if (files_to_process.empty()) { std::cout << YELLOW_COLOR << "警告: 在文件夹 " << input_path_str << " 中未找到 .txt 文件。" << RESET_COLOR << std::endl; return 0; }
        std::sort(files_to_process.begin(), files_to_process.end());
    } else {
        files_to_process.push_back(input_path);
    }
    
    fs::path output_root_path;
    if (is_input_a_directory && convert) {
        // 修正: 正确添加 Processed_ 前缀
        output_root_path = input_path.parent_path() / ("Processed_" + input_path.filename().string());
        try {
            fs::create_directories(output_root_path);
            std::cout << "信息: 已创建输出根目录: " << output_root_path << std::endl;
        } catch (const fs::filesystem_error& e) { std::cerr << RED_COLOR << "致命错误: 无法创建输出根目录: " << e.what() << RESET_COLOR << std::endl; return 1; }
    }

    // =========================================================================
    //                            执行逻辑
    // =========================================================================

    if (run_all) {
        // --- "-a" 模式：三段式处理，错误即停 ---
        std::cout << "\n--- 运行 \"all-in-one\" 模式 (错误即停) ---\n" << std::endl;
        
        FormatValidator validator(validator_config, interval_config, enable_day_count_check);
        std::map<fs::path, fs::path> source_to_output_map;

        // --- 阶段一：全部检验源文件 ---
        std::cout << "--- 阶段 1: 检验所有源文件... ---" << std::endl;
        for (const auto& file : files_to_process) {
            std::set<FormatValidator::Error> errors;
            if (!validator.validateSourceFile(file.string(), errors)) {
                std::cerr << RED_COLOR << "错误: 源文件 " << file.string() << " 检验失败。" << RESET_COLOR << std::endl;
                ErrorReporter::printGroupedErrors(file.string(), errors, error_file);
                std::cerr << "\n程序已终止。" << std::endl;
                return 1;
            }
        }
        std::cout << GREEN_COLOR << "所有源文件检验通过。" << RESET_COLOR << std::endl;

        // --- 阶段二：全部转换 ---
        std::cout << "\n--- 阶段 2: 转换所有文件... ---" << std::endl;
        IntervalProcessor processor(interval_config);
        for (const auto& file : files_to_process) {
            fs::path target_path;
            if (is_input_a_directory) {
                target_path = output_root_path / fs::relative(file, input_path);
                fs::create_directories(target_path.parent_path());
            } else { target_path = "Processed_" + file.filename().string(); }
            
            // 提取年份
            std::string year_str;
            fs::path current_path = file.parent_path();
            auto is_four_digit_string = [](const std::string& s) { return s.length() == 4 && std::all_of(s.begin(), s.end(), ::isdigit); };
            while (!current_path.empty() && current_path.has_filename()) {
                if (is_four_digit_string(current_path.filename().string())) {
                    year_str = current_path.filename().string();
                    break;
                }
                current_path = current_path.parent_path();
            }
             if (year_str.empty()) {
                std::time_t now = std::time(nullptr);
                std::tm* ltm = std::localtime(&now);
                year_str = std::to_string(1900 + ltm->tm_year);
            }

            if (!processor.executeConversion(file.string(), target_path.string(), year_str)) {
                std::cerr << RED_COLOR << "错误: 转换文件 " << file.string() << " 失败。" << RESET_COLOR << std::endl;
                std::cerr << "\n程序已终止。" << std::endl;
                return 1;
            }
            source_to_output_map[file] = target_path;
        }
        std::cout << GREEN_COLOR << "所有文件转换成功。" << RESET_COLOR << std::endl;

        // --- 阶段三：全部检验输出文件 ---
        std::cout << "\n--- 阶段 3: 检验所有输出文件... ---" << std::endl;
        for (const auto& pair : source_to_output_map) {
            const fs::path& output_file = pair.second;
            std::set<FormatValidator::Error> errors;
            if (!validator.validateOutputFile(output_file.string(), errors)) {
                std::cerr << RED_COLOR << "错误: 输出文件 " << output_file.string() << " 检验失败。" << RESET_COLOR << std::endl;
                ErrorReporter::printGroupedErrors(output_file.string(), errors, error_file);
                std::cerr << "\n程序已终止。" << std::endl;
                return 1;
            }
        }
        std::cout << GREEN_COLOR << "所有输出文件检验通过。" << RESET_COLOR << std::endl;
        std::cout << "\n--- 所有流程成功完成！ ---\n" << std::endl;

    } else {
        // --- 普通模式：逐个文件处理 ---
        int v_source_success = 0, v_source_fail = 0;
        int convert_success = 0, convert_fail = 0;
        int v_output_success = 0, v_output_fail = 0;
        
        for (const auto& file : files_to_process) {
            std::cout << "\n=======================================================\n";
            std::cout << "正在处理文件: " << file.string() << "\n";
            
            bool current_file_ok = true;
            fs::path final_output_path;

            if (validate_source) {
                std::cout << "--- 阶段 1: 检验源文件 ---" << std::endl;
                FormatValidator validator(validator_config, interval_config, enable_day_count_check);
                std::set<FormatValidator::Error> errors;
                if (validator.validateSourceFile(file.string(), errors)) { v_source_success++; std::cout << GREEN_COLOR << "成功: 源文件格式合规。" << RESET_COLOR << std::endl; } 
                else { v_source_fail++; current_file_ok = false; std::cerr << RED_COLOR << "失败: 源文件格式错误。" << RESET_COLOR << std::endl; ErrorReporter::printGroupedErrors(file.string(), errors, error_file); }
            }

            if (convert && current_file_ok) {
                std::cout << "--- 阶段 2: 转换文件 ---" << std::endl;
                if (is_input_a_directory) {
                    final_output_path = output_root_path / fs::relative(file, input_path);
                    fs::create_directories(final_output_path.parent_path());
                } else { final_output_path = "Processed_" + file.filename().string(); }

                std::string year_str;
                fs::path current_path = file.parent_path();
                auto is_four_digit_string = [](const std::string& s) { return s.length() == 4 && std::all_of(s.begin(), s.end(), ::isdigit); };
                 while (!current_path.empty() && current_path.has_filename()) {
                    if (is_four_digit_string(current_path.filename().string())) {
                        year_str = current_path.filename().string();
                        break;
                    }
                    current_path = current_path.parent_path();
                }
                if (year_str.empty()) {
                    std::time_t now = std::time(nullptr);
                    std::tm* ltm = std::localtime(&now);
                    year_str = std::to_string(1900 + ltm->tm_year);
                }

                IntervalProcessor processor(interval_config);
                if (processor.executeConversion(file.string(), final_output_path.string(), year_str)) {
                    convert_success++;
                    std::cout << GREEN_COLOR << "成功: 文件已转换为 " << final_output_path.string() << RESET_COLOR << std::endl;
                } else {
                    convert_fail++;
                    current_file_ok = false;
                    std::cerr << RED_COLOR << "失败: 文件转换过程中发生错误。" << RESET_COLOR << std::endl;
                }
            }

            if (validate_output && current_file_ok) {
                std::cout << "--- 阶段 3: 检验输出文件 ---" << std::endl;
                FormatValidator validator(validator_config, interval_config, enable_day_count_check);
                std::set<FormatValidator::Error> errors;
                if (validator.validateOutputFile(final_output_path.string(), errors)) {
                    v_output_success++;
                    std::cout << GREEN_COLOR << "成功: 输出文件格式合规。" << RESET_COLOR << std::endl;
                } else {
                    v_output_fail++;
                    std::cerr << RED_COLOR << "失败: 输出文件 " << final_output_path.string() << " 格式错误。" << RESET_COLOR << std::endl;
                    ErrorReporter::printGroupedErrors(final_output_path.string(), errors, error_file);
                }
            }
        }
        // 最终报告
        std::cout << "\n\n--- 所有任务处理完毕 ---\n";
        if (validate_source) { std::cout << "  - 源文件检验: " << GREEN_COLOR << v_source_success << " 成功" << RESET_COLOR << ", " << RED_COLOR << v_source_fail << " 失败" << RESET_COLOR << std::endl; }
        if (convert) { std::cout << "  - 文件转换:   " << GREEN_COLOR << convert_success << " 成功" << RESET_COLOR << ", " << RED_COLOR << convert_fail << " 失败" << RESET_COLOR << std::endl; }
        if (validate_output) { std::cout << "  - 输出文件检验: " << GREEN_COLOR << v_output_success << " 成功" << RESET_COLOR << ", " << RED_COLOR << v_output_fail << " 失败" << RESET_COLOR << std::endl; }
    }
    
    return 0;
}