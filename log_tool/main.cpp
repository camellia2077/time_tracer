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
        // (Help text omitted for brevity)
        return 1;
    }


    // --- Config File Paths ---
    std::string interval_config = "interval_processor_config.json";
    std::string validator_config = "format_validator_config.json";
    std::string header_config = "header_format.json";
    std::string error_file = "validation_errors.txt";
    
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
    
    auto parsing_end_time = std::chrono::high_resolution_clock::now();
    
    int success_count = 0;
    int failure_count = 0;
    int conversion_success_count = 0;
    int conversion_failure_count = 0;
    auto parsing_duration = parsing_end_time - parsing_start_time;
    auto conversion_duration = std::chrono::high_resolution_clock::duration::zero();

    // --- MODIFICATION START: Add a flag to track error file creation ---
    bool error_file_initialized = false;
    // --- MODIFICATION END ---

    // --- Loop to process all found files ---
    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        std::cout << "正在处理文件: " << file.string() << "\n";
        
        std::string source_filepath = file.string();
        std::string processed_filepath = ""; 
        bool is_valid = true; 

        // --- 阶段一: 文件处理 (-p or -pv) ---
        if (process) {
            std::string initial_output_filename = (validate) 
                ? "temp_" + file.filename().string() 
                : "processed_" + file.filename().string();

            auto conversion_start_time = std::chrono::high_resolution_clock::now();
            IntervalProcessor processor(interval_config, header_config);
            
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
            
            FormatValidator validator(validator_config, header_config, enable_day_count_check);
            std::set<FormatValidator::Error> errors;
            is_valid = validator.validateFile(file_to_validate, errors);

            if (is_valid) {
                std::cout << GREEN_COLOR << "\nSuccess: "<< RESET_COLOR << "文件通过所有有效性检查。" << std::endl;
                success_count++;
            } else {
                std::cerr << RED_COLOR << "\nErrors: " << RESET_COLOR << "在文件中发现错误。"  << std::endl;
                
                // --- MODIFICATION START: Create error file on demand ---
                if (!error_file_initialized) {
                    // 这是本次运行中第一次发现错误，现在创建/清空错误日志文件
                    std::ofstream ofs(error_file, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    error_file_initialized = true; // 标记已创建，避免重复清空
                }
                // --- MODIFICATION END ---

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