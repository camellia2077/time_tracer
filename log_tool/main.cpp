// main.cpp
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

// For platform-specific UTF-8 console setup
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

// Sets up the console to correctly display UTF-8 characters.
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


int main(int argc, char* argv[]) {
    // --- Setup ---
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    //定义版本和日期信息
    const std::string APP_VERSION = "0.0.1";
    const std::string LAST_UPDATE_DATE = "2025-06-24";

    // 添加 --version 标志的检查逻辑 这个检查应该在所有其他参数处理之前
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "版本号: " << APP_VERSION << std::endl;
        std::cout << "最后更新时间: " << LAST_UPDATE_DATE << std::endl;
        return 0; // 打印信息后立即退出
    }

    auto total_start_time = std::chrono::high_resolution_clock::now();
    auto parsing_start_time = std::chrono::high_resolution_clock::now();
    
    // --- Argument Parsing ---
    bool process = false;
    bool validate = false;
    std::string input_path_str;
    bool enable_day_count_check = false; 
    std::string mode_flag;

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    auto it = std::remove_if(args.begin(), args.end(), [&](const std::string& arg) {
        if (arg == "--enable-day-check" || arg == "-edc") {
            enable_day_count_check = true;
            return true; 
        }
        return false;
    });
    args.erase(it, args.end()); 

    if (args.size() != 2) {
        std::cerr << RED_COLOR << "使用方法: " << argv[0] << " <flag> <文件或文件夹路径> [options]" << RESET_COLOR << std::endl;
        std::cerr << "  flags:" << std::endl;
        std::cerr << "    -p\t只读取后转换文件,不检验内容合法性" << std::endl;
        std::cerr << "    -pv\t读取文件转换,并且检验合法性" << std::endl;
        std::cerr << "    -v\t只检验,不转换文件" << std::endl;
        std::cerr << "  options (可选):" << std::endl;
        std::cerr << "    --enable-day-check, -edc\t启用对月份天数完整性的检查 (默认关闭)" << std::endl;
        std::cerr << "  other:" << std::endl;
        std::cerr << "    --version\t显示版本号和更新日期" << std::endl;
        return 1;
    }

    mode_flag = args[0];
    input_path_str = args[1];
    
    if (mode_flag == "-p"|| mode_flag == "-P") {
        process = true;
    } else if (mode_flag == "-pv"|| mode_flag == "-PV") {
        process = true;
        validate = true;
    } else if (mode_flag == "-v"||mode_flag == "-V") {
        validate = true;
    } else {
        std::cerr << RED_COLOR << "Errors: " << RESET_COLOR <<  "未知的 flag '" << mode_flag << "'" << std::endl;
        return 1;
    }
    
    // --- Config File Paths ---
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string error_file = "validation_errors.txt";
    
    // --- File/Directory Path Handling ---
    fs::path input_path(input_path_str);
    std::vector<fs::path> files_to_process;

    if (!fs::exists(input_path)) {
        std::cerr << RED_COLOR << "Errors: "<< RESET_COLOR << "输入的路径不存在: " << input_path_str  << std::endl;
        return 1;
    }

    // --- MODIFICATION START: New logic for directory handling ---
    bool is_input_a_directory = fs::is_directory(input_path);
    fs::path output_root_path; // Will store the new root like "p_Date"

    if (is_input_a_directory) {
        std::cout << "检测到输入为文件夹,将处理其中所有的 .txt 文件..." << std::endl;
        
        // If in pure '-p' mode, create the new root output directory
        if (process && !validate) {
            // Use parent_path() to place the new directory alongside the original
            output_root_path = input_path.parent_path() / ("p_" + input_path.filename().string());
            try {
                fs::create_directories(output_root_path);
                std::cout << "Info: 创建输出目录以保持原始结构: " << output_root_path.string() << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << RED_COLOR << "FATAL: " << RESET_COLOR << "创建目录失败: " << output_root_path.string() << ". 原因: " << e.what() << std::endl;
                return 1;
            }
        }

        for (const auto& entry : fs::recursive_directory_iterator(input_path)) {
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
    // --- MODIFICATION END ---
    
    auto parsing_end_time = std::chrono::high_resolution_clock::now();
    
    int success_count = 0;
    int failure_count = 0;
    int conversion_success_count = 0;
    int conversion_failure_count = 0;
    auto parsing_duration = parsing_end_time - parsing_start_time;
    auto conversion_duration = std::chrono::high_resolution_clock::duration::zero();

    bool error_file_initialized = false;

    // --- Loop to process all found files ---
    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        std::cout << "正在处理文件: " << file.string() << "\n";
        
        std::string source_filepath = file.string();
        std::string processed_filepath = ""; 
        bool is_valid = true; 

        // --- 阶段一: 文件处理 (-p or -pv) ---
        if (process) {
            std::string initial_output_filename;

            // --- MODIFICATION START: Determine output path based on input type ---
            // If it's a directory under pure '-p' mode, construct the mirrored path
            if (is_input_a_directory && !validate) {
                try {
                    fs::path relative_path = fs::relative(file, input_path);
                    fs::path final_output_path = output_root_path / relative_path;

                    // Ensure the target subdirectory (e.g., p_Date/2023/) exists before writing the file
                    fs::create_directories(final_output_path.parent_path());
                    
                    initial_output_filename = final_output_path.string();
                } catch (const fs::filesystem_error& e) {
                     std::cerr << RED_COLOR << "Errors: " << RESET_COLOR << "无法创建输出目录结构。原因: " << e.what() << ". 跳过此文件。" << std::endl;
                     conversion_failure_count++;
                     continue;
                }
            } else {
                // Original logic for single files or modes other than '-p'
                 initial_output_filename = (validate) 
                    ? "temp_" + file.filename().string() 
                    : "processed_" + file.filename().string();
            }
            // --- MODIFICATION END ---

            auto conversion_start_time = std::chrono::high_resolution_clock::now();
            IntervalProcessor processor(interval_config);
            
            if (!processor.processFile(source_filepath, initial_output_filename)) {
                std::cerr << RED_COLOR << "Errors: " << RESET_COLOR << "处理文件失败。跳过此文件。" << std::endl;
                conversion_failure_count++;
                std::cout << "=======================================================\n";
                continue; 
            }
            
            processed_filepath = initial_output_filename; 
            conversion_success_count++; 
            auto conversion_end_time = std::chrono::high_resolution_clock::now();
            conversion_duration += conversion_end_time - conversion_start_time;

            if (!validate) {
                 std::cout << GREEN_COLOR << "Succeeded: " << RESET_COLOR << "文件转换完成。输出文件为: " << processed_filepath << std::endl;
            }
        }

        // --- 阶段二: 文件校验 (-v or -pv) ---
        if (validate) {
            std::string file_to_validate = (process) ? processed_filepath : source_filepath;
            
            FormatValidator validator(validator_config, interval_config, enable_day_count_check);
            std::set<FormatValidator::Error> errors;
            is_valid = validator.validateFile(file_to_validate, errors);

            if (is_valid) {
                std::cout << GREEN_COLOR << "\nSuccess: "<< RESET_COLOR << "文件通过所有有效性检查。" << std::endl;
                success_count++;
            } else {
                std::cerr << RED_COLOR << "\nErrors: " << RESET_COLOR << "在文件中发现错误。"  << std::endl;
                
                if (!error_file_initialized) {
                    std::ofstream ofs(error_file, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    error_file_initialized = true; 
                }

                ErrorReporter::printGroupedErrors(file_to_validate, errors, error_file);
                failure_count++;
            }
        }
        
        // --- 阶段三: 文件重命名 (仅用于 -pv 模式) ---
        if (process && validate) {
            std::string final_filename;
            if (is_valid) {
                final_filename = "final_" + file.filename().string();
                std::cout << GREEN_COLOR << "Info: " << RESET_COLOR << "检验成功，文件重命名为: " << final_filename << std::endl;
            } else {
                final_filename = "error_validation_" + file.filename().string();
                std::cout << YELLOW_COLOR << "Info: " << RESET_COLOR << "检验失败，文件重命名为: " << final_filename << std::endl;
            }

            try {
                if (fs::exists(final_filename)) {
                    fs::remove(final_filename);
                }
                fs::rename(processed_filepath, final_filename);
            } catch (const fs::filesystem_error& e) {
                std::cerr << RED_COLOR << "FATAL: " << RESET_COLOR << "无法重命名文件 " << processed_filepath << " 到 " << final_filename << "。原因: " << e.what() << std::endl;
            }
        }

        std::cout << "=======================================================\n";
    }
    
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = total_end_time - total_start_time;
    double total_seconds = std::chrono::duration<double>(total_duration).count();
    double parsing_seconds = std::chrono::duration<double>(parsing_duration).count();
    double conversion_seconds = std::chrono::duration<double>(conversion_duration).count();


    // --- Final Output ---
    std::cout << "\n--- 所有任务处理完毕 ---" << std::endl;
    
    std::cout << "--------------------------------------";
    std::cout << "\nTiming Statistics:\n\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Total time: " << total_seconds << " seconds (" << total_seconds * 1000.0 << " ms)\n";
    std::cout << "  - Parsing files: " << parsing_seconds << " seconds (" << parsing_seconds * 1000.0 << " ms)\n";
    if (process) {
        std::cout << "  - File conversion: " << conversion_seconds << " seconds (" << conversion_seconds * 1000.0 << " ms)\n";
    }
    std::cout << "--------------------------------------";

    std::cout << "\n格式转换成功的txt数量:" << conversion_success_count << std::endl;
    std::cout << "格式转换失败的txt数量:" << conversion_failure_count << std::endl;
    std::cout  << std::endl;
    std::cout << "检验成功的txt数量:" << success_count << std::endl;
    std::cout << "检验失败的txt数量:" << failure_count << std::endl;

    return 0;
}