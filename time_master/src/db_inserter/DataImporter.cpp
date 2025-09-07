// db_inserter/DataImporter.cpp
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <fstream>

#include "common/AnsiColors.hpp" // 获取颜色宏定义

#include "db_inserter/parser/JsonDataParser.hpp"
#include "db_inserter/inserter/DatabaseInserter.hpp"
          
namespace fs = std::filesystem;

class UserInteractor {
public:
    std::vector<std::string> collect_paths_from_user() const {
        std::cout << "Enter .json file(s) or directory path(s) to process: ";
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

class PathScanner {
public:
    std::vector<std::string> find_json_files(const std::vector<std::string>& input_paths) const {
        std::vector<std::string> files_to_process;
        for (const std::string& path_str : input_paths) {
            fs::path p(path_str);
            if (!fs::exists(p)) {
                std::cerr << "Warning: Path does not exist: " << path_str << std::endl;
                continue;
            }
            if (fs::is_regular_file(p) && p.extension() == ".json") {
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
                if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".json") {
                    files.push_back(entry.path().string());
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error accessing directory " << dir_path << ": " << e.what() << std::endl;
        }
    }
};

class ProcessReporter {
public:
    struct TimingResult { double parsing_s = 0.0; double db_insertion_s = 0.0; };
    void report_results(size_t total_file_count, const std::vector<std::string>& failed_files, const TimingResult& timing) const {
        size_t successful_count = total_file_count - failed_files.size();
        double total_time = timing.parsing_s + timing.db_insertion_s;
        std::cout << "\n--- Data processing complete. ---" << std::endl;
        if (failed_files.empty()) {
            std::cout << GREEN_COLOR << "All files successfully processed and imported." << RESET_COLOR << std::endl;
            std::cout << "Successfully processed " << successful_count << " files." << std::endl;
        } else {
            std::cerr << "There were errors during the parsing stage." << std::endl;
            if (successful_count > 0) std::cout << "Successfully parsed " << successful_count << " files." << std::endl;
            std::cerr << "Failed to parse the following " << failed_files.size() << " files:" << std::endl;
            for (const std::string& fname : failed_files) std::cerr << "- " << fname << std::endl;
        }
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "\n--------------------------------------\n" << "Timing Statistics:\n" << std::endl;
        std::cout << "Total time: " << total_time << " seconds (" << total_time * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Parsing files: " << timing.parsing_s << " seconds (" << timing.parsing_s * 1000.0 << " ms)" << std::endl;
        std::cout << "  - Database insertion: " << timing.db_insertion_s << " seconds (" << timing.db_insertion_s * 1000.0 << " ms)" << std::endl;
        std::cout << "--------------------------------------\n";
    }
};

class ProcessOrchestrator {
public:
    // [核心修改] 构造函数不再需要 config_path
    explicit ProcessOrchestrator(std::string db_name) 
        : db_name_(std::move(db_name)) {}

    void run(const std::vector<std::string>& files_to_process) {
        if (files_to_process.empty()) {
            std::cout << "No .json files found to process." << std::endl;
            return;
        }
        std::cout << "\nStart processing " << files_to_process.size() << " file(s)... " << std::endl;
        ProcessReporter::TimingResult timing;
        auto start_total = std::chrono::high_resolution_clock::now();

        std::cout << "Stage 1: Parsing JSON files into memory..." << std::endl;
        // [核心修改] 直接创建新的 JsonDataParser
        JsonDataParser parser;

        std::vector<std::string> failed_files = parse_all_files(parser, files_to_process);
        auto end_parsing = std::chrono::high_resolution_clock::now();
        timing.parsing_s = std::chrono::duration<double>(end_parsing - start_total).count();

        std::cout << "Stage 2: Importing data into the database..." << std::endl;
        DatabaseInserter inserter(db_name_);
        if (inserter.is_db_open()) {
            inserter.import_data(parser.days, parser.records, parser.parent_child_pairs);
        } else {
            std::cerr << "Inserter could not open database. Aborting import." << std::endl;
        }
        auto end_total = std::chrono::high_resolution_clock::now();
        timing.db_insertion_s = std::chrono::duration<double>(end_total - end_parsing).count();
        
        ProcessReporter reporter;
        reporter.report_results(files_to_process.size(), failed_files, timing);
    }

private:
    std::vector<std::string> parse_all_files(JsonDataParser& parser, const std::vector<std::string>& files) {
        std::vector<std::string> failed_files;
        for (const std::string& fname : files) {
            if (!parser.parse_file(fname)) {
                failed_files.push_back(fname);
            }
        }
        return failed_files;
    }
    std::string db_name_;
};

// [核心修改] 接口不再需要 config_path
void handle_process_files(const std::string& db_name, const std::string& config_path) {
    UserInteractor interactor;
    std::vector<std::string> user_paths = interactor.collect_paths_from_user();
    PathScanner scanner;
    std::vector<std::string> files_to_process = scanner.find_json_files(user_paths);
    ProcessOrchestrator orchestrator(db_name);
    orchestrator.run(files_to_process);
}

void handle_process_files(const std::string& db_name, const std::string& path, const std::string& config_path) {
    PathScanner scanner;
    std::vector<std::string> files_to_process = scanner.find_json_files({path});
    ProcessOrchestrator orchestrator(db_name);
    orchestrator.run(files_to_process);
}