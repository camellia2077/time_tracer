// main.cpp (已修正以支持输出目录结构)
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
    std::cerr << "\n必须至少选择以下一个主要动作标志:" << std::endl;
    std::cerr << "  --convert\t\t转换文件格式。" << std::endl;
    std::cerr << "  --validate-source\t检验源文件的格式。" << std::endl;
    
    std::cerr << "\n可选的附加标志:" << std::endl;
    std::cerr << "  --validate-output\t转换后检验输出文件 (必须与 --convert 一起使用)。" << std::endl;
    std::cerr << "  --enable-day-check, -edc\t启用对月份天数完整性的检查。" << std::endl;
    
    std::cerr << "\n其他:" << std::endl;
    std::cerr << "  --version\t\t显示版本号和更新日期。" << std::endl;
}


int main(int argc, char* argv[]) {
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    const std::string APP_VERSION = "0.2.0"; // Version updated for new output structure
    const std::string LAST_UPDATE_DATE = "2025-06-30"; 

    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "版本号: " << APP_VERSION << std::endl;
        std::cout << "最后更新时间: " << LAST_UPDATE_DATE << std::endl;
        return 0;
    }
    
    auto total_start_time = std::chrono::high_resolution_clock::now();
    
    // --- 新的参数解析逻辑 ---
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
    std::string input_path_str;

    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        print_usage(argv[0]);
        return 1;
    }
    
    // 分离路径和标志
    for (const auto& arg : args) {
        if (arg == "--convert") convert = true;
        else if (arg == "--validate-source") validate_source = true;
        else if (arg == "--validate-output") validate_output = true;
        else if (arg == "--enable-day-check" || arg == "-edc") enable_day_count_check = true;
        else if (arg.rfind("-", 0) != 0) { // 如果参数不以'-'开头，则认为是路径
            if (!input_path_str.empty()) {
                std::cerr << RED_COLOR << "错误: 检测到多个输入路径。请只提供一个。" << RESET_COLOR << std::endl;
                return 1;
            }
            input_path_str = arg;
        }
    }

    if (input_path_str.empty()) {
        std::cerr << RED_COLOR << "错误: 未提供输入文件或文件夹路径。" << RESET_COLOR << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    if (!convert && !validate_source) {
        std::cerr << RED_COLOR << "错误: 必须指定至少一个主要动作 (--convert 或 --validate-source)。" << RESET_COLOR << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    if (validate_output && !convert) {
        std::cerr << RED_COLOR << "错误: --validate-output 必须与 --convert 一起使用。" << RESET_COLOR << std::endl;
        return 1;
    }
    
    // --- 配置和文件处理 ---
    auto parsing_start_time = std::chrono::high_resolution_clock::now();
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string error_file = "validation_errors.txt";
    
    fs::path input_path(input_path_str);
    std::vector<fs::path> files_to_process;

    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << input_path_str  << std::endl;
        return 1;
    }

    bool is_input_a_directory = fs::is_directory(input_path);
    if (is_input_a_directory) {
        std::cout << "检测到输入为文件夹, 将处理其中所有的 .txt 文件..." << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                files_to_process.push_back(entry.path());
            }
        }
        if (files_to_process.empty()) {
            std::cout << YELLOW_COLOR << "警告: 在文件夹 " << input_path_str << " 中未找到 .txt 文件。" << std::endl;
            return 0;
        }
        std::sort(files_to_process.begin(), files_to_process.end());
    } else {
        files_to_process.push_back(input_path);
    }
    
    auto parsing_end_time = std::chrono::high_resolution_clock::now();
    
    // --- 新增: 创建主输出目录 ---
    fs::path output_root_path;
    if (is_input_a_directory && convert) {
        output_root_path = input_path.parent_path() / ("Processed_" + input_path.filename().string());
        try {
            fs::create_directories(output_root_path);
            std::cout << "信息: 已创建输出根目录: " << output_root_path << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << RED_COLOR << "致命错误: 无法创建输出根目录: " << e.what() << RESET_COLOR << std::endl;
            return 1;
        }
    }

    int v_source_success = 0, v_source_fail = 0;
    int convert_success = 0, convert_fail = 0;
    int v_output_success = 0, v_output_fail = 0;
    auto conversion_duration = std::chrono::high_resolution_clock::duration::zero();
    bool error_file_initialized = false;

    // --- 主处理循环 ---
    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        std::cout << "正在处理文件: " << file.string() << "\n";
        
        bool current_file_ok = true;
        std::string processed_filepath = ""; // 将保存临时文件的完整路径

        // 阶段一: 检验源文件 (如果请求)
        if (validate_source) {
            std::cout << "--- 阶段 1: 检验源文件 ---" << std::endl;
            IntervalProcessor source_validator(interval_config);
            if (source_validator.validateFile(file.string())) {
                std::cout << GREEN_COLOR << "成功: 源文件格式合规。" << RESET_COLOR << std::endl;
                v_source_success++;
            } else {
                std::cerr << RED_COLOR << "失败: 源文件基本格式错误。详细信息请见上方日志。" << RESET_COLOR << std::endl;
                v_source_fail++;
                current_file_ok = false;
            }
        }

        // 阶段二: 转换文件 (如果请求且源文件OK)
        if (convert) {
            if (!current_file_ok) {
                std::cout << YELLOW_COLOR << "跳过: 由于源文件检验失败，已跳过转换步骤。" << RESET_COLOR << std::endl;
                convert_fail++;
            } else {
                std::cout << "--- 阶段 2: 转换文件 ---" << std::endl;
                
                // 从路径提取年份
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

                // --- 核心修改: 构建包含目录结构的输出路径 ---
                fs::path target_path;
                if (is_input_a_directory) {
                    fs::path relative_path = fs::relative(file, input_path);
                    target_path = output_root_path / relative_path;
                } else {
                    target_path = file.filename(); // 单文件情况，在当前目录输出
                }

                try {
                    fs::create_directories(target_path.parent_path());
                } catch(const fs::filesystem_error& e) {
                    std::cerr << RED_COLOR << "错误: 创建子目录失败: " << e.what() << RESET_COLOR << std::endl;
                    convert_fail++;
                    continue; // 跳过这个文件
                }

                std::string temp_output_filename = (target_path.parent_path() / ("temp_" + target_path.filename().string())).string();
                
                auto conv_start = std::chrono::high_resolution_clock::now();
                IntervalProcessor processor(interval_config);
                
                if (processor.executeConversion(file.string(), temp_output_filename, year_str)) {
                    std::cout << GREEN_COLOR << "成功: 文件已转换为临时文件: " << temp_output_filename << RESET_COLOR << std::endl;
                    processed_filepath = temp_output_filename;
                    convert_success++;
                } else {
                    std::cerr << RED_COLOR << "失败: 文件转换过程中发生错误。" << RESET_COLOR << std::endl;
                    convert_fail++;
                    current_file_ok = false;
                }
                conversion_duration += std::chrono::high_resolution_clock::now() - conv_start;
            }
        }

        // 阶段三: 检验输出文件 (如果请求且转换成功)
        if (validate_output) {
            if (!processed_filepath.empty()) {
                std::cout << "--- 阶段 3: 检验输出文件 ---" << std::endl;
                FormatValidator validator(validator_config, interval_config, enable_day_count_check);
                std::set<FormatValidator::Error> errors;
                bool output_is_valid = validator.validateFile(processed_filepath, errors);
                
                fs::path temp_path(processed_filepath);
                std::string final_filename_str;
                if (output_is_valid) {
                    final_filename_str = (temp_path.parent_path() / ("final_" + file.filename().string())).string();
                    std::cout << GREEN_COLOR << "成功: 输出文件格式合规。重命名为: " << final_filename_str << RESET_COLOR << std::endl;
                    v_output_success++;
                } else {
                    final_filename_str = (temp_path.parent_path() / ("error_validation_" + file.filename().string())).string();
                    std::cerr << RED_COLOR << "失败: 输出文件格式错误。重命名为: " << final_filename_str << RESET_COLOR << std::endl;
                    v_output_fail++;
                    if (!error_file_initialized) {
                         std::ofstream ofs(error_file, std::ofstream::out | std::ofstream::trunc);
                         ofs.close();
                         error_file_initialized = true;
                    }
                    ErrorReporter::printGroupedErrors(processed_filepath, errors, error_file);
                }

                try {
                    if (fs::exists(final_filename_str)) fs::remove(final_filename_str);
                    fs::rename(processed_filepath, final_filename_str);
                } catch (const fs::filesystem_error& e) {
                    std::cerr << RED_COLOR << "致命错误: 无法重命名文件 " << processed_filepath << " 到 " << final_filename_str << "。原因: " << e.what() << RESET_COLOR << std::endl;
                }
            }
        } else if (!processed_filepath.empty()) {
             // 如果只转换不检验输出，则直接重命名
             fs::path temp_path(processed_filepath);
             std::string final_filename_str = (temp_path.parent_path() / ("processed_" + file.filename().string())).string();
             try {
                    if (fs::exists(final_filename_str)) fs::remove(final_filename_str);
                    fs::rename(processed_filepath, final_filename_str);
                    std::cout << "信息: 文件已保存为: " << final_filename_str << std::endl;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << RED_COLOR << "致命错误: 无法重命名文件 " << processed_filepath << " 到 " << final_filename_str << "。原因: " << e.what() << RESET_COLOR << std::endl;
                }
        }
    }
    
    // --- 最终统计报告 ---
    auto total_end_time = std::chrono::high_resolution_clock::now();
    double total_seconds = std::chrono::duration<double>(total_end_time - total_start_time).count();
    double parsing_seconds = std::chrono::duration<double>(parsing_end_time - parsing_start_time).count();
    double conversion_seconds = std::chrono::duration<double>(conversion_duration).count();

    std::cout << "\n\n--- 所有任务处理完毕 ---" << std::endl;
    std::cout << "--------------------------------------\n";
    std::cout << "计时统计:\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  - 总耗时: " << total_seconds << " 秒\n";
    std::cout << "  - 文件解析耗时: " << parsing_seconds << " 秒\n";
    if (convert) {
        std::cout << "  - 文件转换耗时: " << conversion_seconds << " 秒\n";
    }
    std::cout << "--------------------------------------\n";

    std::cout << "最终报告:\n";
    if (validate_source) {
        std::cout << "  - 源文件检验: " << GREEN_COLOR << v_source_success << " 成功" << RESET_COLOR << ", "
                  << RED_COLOR << v_source_fail << " 失败" << RESET_COLOR << std::endl;
    }
    if (convert) {
        std::cout << "  - 文件转换:   " << GREEN_COLOR << convert_success << " 成功" << RESET_COLOR << ", "
                  << RED_COLOR << convert_fail << " 失败" << RESET_COLOR << std::endl;
    }
    if (validate_output) {
         std::cout << "  - 输出文件检验: " << GREEN_COLOR << v_output_success << " 成功" << RESET_COLOR << ", "
                  << RED_COLOR << v_output_fail << " 失败" << RESET_COLOR << std::endl;
    }
    if (v_source_fail > 0 || v_output_fail > 0) {
        std::cout << "\n注意: " << YELLOW_COLOR << "发现错误，详情请查看文件 " << error_file << RESET_COLOR << std::endl;
    }

    return 0;
}