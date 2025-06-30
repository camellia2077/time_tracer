#include "processing.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <chrono>

// 包含项目内其他模块的头文件
#include "common_utils.h"
#include "data_parser.h"
#include "database_inserter.h"

// 定义命名空间别名
namespace fs = std::filesystem;

// =================================================================
// Class: UserInteractor (负责用户交互)
// =================================================================
class UserInteractor {
public:
    std::vector<std::string> collect_paths_from_user() const {
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
};

// =================================================================
// Class: PathScanner (仅负责文件查找)
// =================================================================
class PathScanner {
public:
    std::vector<std::string> find_txt_files(const std::vector<std::string>& input_paths) const {
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
                find_in_directory(p, files_to_process);
            }
        }
        std::sort(files_to_process.begin(), files_to_process.end());
        return files_to_process;
    }

private:
    void find_in_directory(const fs::path& dir_path, std::vector<std::string>& files) const {
        try {
            for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
                if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".txt") {
                    files.push_back(entry.path().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error accessing directory " << dir_path << ": " << e.what() << std::endl;
        }
    }
};

// =================================================================
// Class: ProcessReporter (仅负责结果报告)
// =================================================================
class ProcessReporter {
public:
    struct TimingResult {
        double parsing_s = 0.0;
        double db_insertion_s = 0.0;
    };

    void report_results(size_t total_file_count, const std::vector<std::string>& failed_files, const TimingResult& timing) const {
        size_t successful_count = total_file_count - failed_files.size();
        double total_time = timing.parsing_s + timing.db_insertion_s;

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
        std::cout << "  - Parsing files: " << timing.parsing_s << " seconds (" << timing.parsing_s * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Database insertion: " << timing.db_insertion_s << " seconds (" << timing.db_insertion_s * 1000.0 << " ms)" << std::endl;
        std::cout << "--------------------------------------\n";
    }
};

// =================================================================
// Class: ProcessOrchestrator (负责流程编排和计时)
// =================================================================
class ProcessOrchestrator {
public:
    explicit ProcessOrchestrator(std::string db_name) : db_name_(std::move(db_name)) {}

    void run(const std::vector<std::string>& files_to_process) {
        if (files_to_process.empty()) {
            std::cout << "No .txt files found to process." << std::endl;
            return;
        }

        std::cout << "\nStart processing " << files_to_process.size() << " file(s)... " << std::endl;
        
        ProcessReporter::TimingResult timing;
        auto start_total = std::chrono::high_resolution_clock::now();

        // Stage 1: Parsing
        std::cout << "Stage 1: Parsing files into memory..." << std::endl;
        DataFileParser parser;
        std::vector<std::string> failed_files = parse_all_files(parser, files_to_process);
        auto end_parsing = std::chrono::high_resolution_clock::now();
        timing.parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();

        // Stage 2: Importing
        std::cout << "Stage 2: Importing data into the database..." << std::endl;
        DatabaseInserter inserter(db_name_);
        if (inserter.is_db_open()) {
            inserter.import_data(parser);
        } else {
            std::cerr << "Inserter could not open database. Aborting import." << std::endl;
        }
        auto end_total = std::chrono::high_resolution_clock::now();
        timing.db_insertion_s = std::chrono::duration<double>(end_total - end_parsing).count();
        
        // Report final results
        ProcessReporter reporter;
        reporter.report_results(files_to_process.size(), failed_files, timing);
    }

private:
    std::vector<std::string> parse_all_files(DataFileParser& parser, const std::vector<std::string>& files) {
        std::vector<std::string> failed_files;
        for (const std::string& fname : files) {
            if (!parser.parse_file(fname)) {
                failed_files.push_back(fname);
            }
        }
        parser.commit_all();
        return failed_files;
    }

    std::string db_name_;
};


// =================================================================
// 函数封装
// =================================================================

/**
 * @brief [交互模式] 实现
 */
void handle_process_files(const std::string& db_name) {
    UserInteractor interactor;
    std::vector<std::string> user_paths = interactor.collect_paths_from_user();
    
    PathScanner scanner;
    std::vector<std::string> files_to_process = scanner.find_txt_files(user_paths);
    
    ProcessOrchestrator orchestrator(db_name);
    orchestrator.run(files_to_process);
}

/**
 * @brief [命令行模式] 重载实现
 */
void handle_process_files(const std::string& db_name, const std::string& path) {
    PathScanner scanner;
    std::vector<std::string> files_to_process = scanner.find_txt_files({path});

    ProcessOrchestrator orchestrator(db_name);
    orchestrator.run(files_to_process);
}