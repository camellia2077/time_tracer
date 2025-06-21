// main.cpp
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <clocale>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono> // 新增：用于计时
#include <iomanip> // 新增：用于格式化输出

#include "IntervalProcessor.h"
#include "FormatValidator.h"
#include "SharedUtils.h"
#include "ErrorReporter.h" // <-- Include the new header

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

// --- REMOVED: getErrorTypeHeader and printGroupedErrors are now in ErrorReporter.cpp ---

int main(int argc, char* argv[]) {
    // --- Setup ---
    setup_console_for_utf8();
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // --- MODIFICATION START: Initialize timers ---
    auto total_start_time = std::chrono::high_resolution_clock::now();
    auto parsing_start_time = std::chrono::high_resolution_clock::now();
    // --- MODIFICATION END ---

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
    
    // --- MODIFICATION START: Stop parsing timer ---
    auto parsing_end_time = std::chrono::high_resolution_clock::now();
    // --- MODIFICATION END ---

    // --- Initialize Counters & Durations ---
    int success_count = 0;
    int failure_count = 0;
    int conversion_success_count = 0;
    int conversion_failure_count = 0;
    // --- MODIFICATION START: Declare duration variables ---
    auto parsing_duration = parsing_end_time - parsing_start_time;
    auto conversion_duration = std::chrono::high_resolution_clock::duration::zero();
    // --- MODIFICATION END ---


    // --- Loop to process all found files ---
    for (const auto& file : files_to_process) {
        std::cout << "\n=======================================================\n";
        std::cout << "正在处理文件: " << file.string() << "\n";
        
        std::string file_to_validate = file.string();
        bool processing_successful = true;

        if (process) {
            // --- MODIFICATION START: Time the conversion block ---
            auto conversion_start_time = std::chrono::high_resolution_clock::now();
            
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

            auto conversion_end_time = std::chrono::high_resolution_clock::now();
            conversion_duration += conversion_end_time - conversion_start_time;
            // --- MODIFICATION END ---
        }

        if (!processing_successful) {
            std::cout << "=======================================================\n";
            continue;
        }

        if (validate) {
            FormatValidator validator(validator_config, header_config);
            std::set<FormatValidator::Error> errors;
            bool is_valid = validator.validateFile(file_to_validate, errors);

            if (is_valid) {
                std::cout << GREEN_COLOR << "\nSuccess: "<< RESET_COLOR << "This file has passed all validity checks." << std::endl;
                success_count++;
            } else {
                std::cerr << RED_COLOR << "\nErrors: " << RESET_COLOR << "Mistakes were found in the file."  << std::endl;
                // --- UPDATED: Call the namespaced function ---
                ErrorReporter::printGroupedErrors(file_to_validate, errors, error_file);
                failure_count++;
            }
        }

        std::cout << "=======================================================\n";
    }
    
    // --- MODIFICATION START: Stop total timer and prepare for final output ---
    auto total_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = total_end_time - total_start_time;

    double total_seconds = std::chrono::duration<double>(total_duration).count();
    double parsing_seconds = std::chrono::duration<double>(parsing_duration).count();
    double conversion_seconds = std::chrono::duration<double>(conversion_duration).count();
    // --- MODIFICATION END ---


    // --- Final Output ---
    std::cout << "\n--- 所有任务处理完毕 ---" << std::endl;
    
    // --- MODIFICATION START: Print Timing Statistics ---
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