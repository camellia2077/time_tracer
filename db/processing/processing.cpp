#include "processing.h"

// 包含所有这个模块需要的标准库头文件
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <chrono>

// 包含项目内其他模块的头文件
#include "common_utils.h"      // 为了使用 ANSI 颜色代码
#include "data_parser.h"       // FileProcessor 依赖于 DataFileParser
#include "database_importer.h" // FileProcessor 依赖于 DatabaseImporter

// 定义命名空间别名
namespace fs = std::filesystem;

// =================================================================
// Class: FileFinder
// (这个类的实现现在完全位于 processing.cpp 内部)
// =================================================================
class FileFinder {
public:
    FileFinder() = default;

    std::vector<std::string> collect_and_find_files() {
        std::vector<std::string> user_inputs = get_user_inputs();
        if (user_inputs.empty()) {
            std::cout << "No filenames or directories entered." << std::endl;
            return {};
        }

        std::vector<std::string> found_files = find_txt_files_in_paths(user_inputs);
        std::sort(found_files.begin(), found_files.end());
        return found_files;
    }

private:
    std::vector<std::string> get_user_inputs() {
        std::cout << "Enter file name(s) or directory path(s) to process (space-separated, then Enter): ";
        std::string line;
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> user_inputs;
        while (ss >> token) {
            user_inputs.push_back(token);
        }
        return user_inputs;
    }

    std::vector<std::string> find_txt_files_in_paths(const std::vector<std::string>& input_paths) {
        std::vector<std::string> files_to_process;
        for (const std::string& path_str : input_paths) {
            fs::path p(path_str);
            if (!fs::exists(p)) {
                std::cerr << "Warning: Path does not exist: " << path_str << std::endl;
                continue;
            }

            if (fs::is_regular_file(p) && p.extension() == ".txt") {
                files_to_process.push_back(p.string());
            } else if (fs::is_directory(p)) {
                try {
                    for (const auto& entry : fs::recursive_directory_iterator(p)) {
                        if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".txt") {
                            files_to_process.push_back(entry.path().string());
                        }
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error accessing directory " << path_str << ": " << e.what() << std::endl;
                }
            }
        }
        return files_to_process;
    }
};


// =================================================================
// Class: FileProcessor
// (这个类的实现现在也完全位于 processing.cpp 内部)
// =================================================================
class FileProcessor {
public:
    explicit FileProcessor(std::string db_name) : db_name_(std::move(db_name)) {}

    void run() {
        FileFinder finder;
        std::vector<std::string> files_to_process = finder.collect_and_find_files();

        if (files_to_process.empty()) {
            std::cout << "No .txt files found to process." << std::endl;
            return;
        }

        std::cout << "\nStart processing " << files_to_process.size() << " file(s)... " << std::endl;
        auto start_total = std::chrono::high_resolution_clock::now();

        // Stage 1: Parsing
        std::cout << "Stage 1: Parsing files into memory..." << std::endl;
        DataFileParser parser;
        std::vector<std::string> failed_files;
        for (const std::string& fname : files_to_process) {
            if (!parser.parse_file(fname)) {
                failed_files.push_back(fname);
            }
        }
        parser.commit_all();
        auto end_parsing = std::chrono::high_resolution_clock::now();

        // Stage 2: Importing
        std::cout << "Stage 2: Importing data into the database..." << std::endl;
        DatabaseImporter importer(db_name_);
        if (!importer.is_db_open()) {
            std::cerr << "Importer could not open database. Aborting." << std::endl;
            double parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();
            report_results(files_to_process.size(), failed_files, parsing_s, 0.0);
            return;
        }
        importer.import_data(parser);
        auto end_total = std::chrono::high_resolution_clock::now();

        // Calculate timings
        double parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();
        double db_insertion_s = std::chrono::duration<double>(end_total - end_parsing).count();

        // Report final results
        report_results(files_to_process.size(), failed_files, parsing_s, db_insertion_s);
    }

private:
    void report_results(size_t total_file_count, const std::vector<std::string>& failed_files, double parsing_time, double db_time) {
        size_t successful_count = total_file_count - failed_files.size();
        double total_time = parsing_time + db_time;

        std::cout << "\n--- Data processing complete. ---" << std::endl;
        if (failed_files.empty()) {
            std::cout << ANSI_COLOR_GREEN << "All files successfully processed and imported." << ANSI_COLOR_RESET << std::endl;
            std::cout << "Successfully processed " << successful_count << " files." << std::endl;
        } else {
            std::cerr << "There were errors during the parsing stage." << std::endl;
            if (successful_count > 0) {
                std::cout << "Successfully parsed " << successful_count << " files." << std::endl;
            }
            std::cerr << "Failed to parse the following " << failed_files.size() << " files:" << std::endl;
            for (const std::string& fname : failed_files) {
                std::cerr << "- " << fname << std::endl;
            }
        }

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n--------------------------------------\n";
        std::cout << "Timing Statistics:\n" << std::endl;
        std::cout << "Total time: " << total_time << " seconds (" << total_time * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Parsing files: " << parsing_time << " seconds (" << parsing_time * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Database insertion: " << db_time << " seconds (" << db_time * 1000.0 << " ms)" << std::endl;
        std::cout << "--------------------------------------\n";
    }

    std::string db_name_;
};

// =================================================================
// Main Application Logic (Implementation)
// =================================================================

/**
 * @brief Simplified function to handle the file processing workflow.
 */
void handle_process_files(const std::string& db_name) {
    FileProcessor processor(db_name);
    processor.run();
}