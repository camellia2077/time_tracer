#include "common/pch.h"
#include "CliController.h"
#include "action_handler/ActionHandler.h"
#include "file_handler/FileController.h"
#include "common/common_utils.h"

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include "action_handler/file/FilePipelineManager.h"

const std::string DATABASE_NAME = "time_data.db";

CliController::CliController(const std::vector<std::string>& args) : args_(args) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = args_[1];

    file_controller_ = new FileController(args_[0]);
    action_handler_ = new ActionHandler(
        DATABASE_NAME,
        file_controller_->get_config(),
        file_controller_->get_main_config_path()
    );
}

CliController::~CliController() {
    delete action_handler_;
    delete file_controller_;
}

void CliController::execute() {
    // --- 新的命令分派結構 ---
    if (command_ == "run-all") {
        handle_run_all();
    } else if (command_ == "preprocess") {
        handle_preprocess();
    } else if (command_ == "import") {
        handle_database_import();
    } else if (command_ == "query") {
        handle_query();
    } else if (command_ == "export") {
        handle_export();
    } else {
        throw std::runtime_error("Unknown command '" + command_ + "'.");
    }
}

// [新] 處理 'run-all' 命令
void CliController::handle_run_all() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command 'run-all' requires exactly one source directory path argument.");
    }
    action_handler_->run_full_pipeline_and_import(args_[2]);
}

// [重構] 處理 'preprocess' 命令
void CliController::handle_preprocess() {
    bool convert_flag = false, validate_source_flag = false, validate_output_flag = false, day_check_flag = false;
    std::string input_path;

    for (size_t i = 2; i < args_.size(); ++i) { // 从 command 之后开始解析
        const std::string& arg = args_[i];
        if (arg == "-c" || arg == "--convert") convert_flag = true;
        else if (arg == "-vs" || arg == "--validate-source") validate_source_flag = true;
        else if (arg == "-vo" || arg == "--validate-output") validate_output_flag = true;
        else if (arg == "-edc" || arg == "--enable-day-check") day_check_flag = true;
        else if (arg.rfind("-", 0) != 0) { // 不是以 '-' 开头的参数被认为是路径
            if (!input_path.empty()) throw std::runtime_error("Multiple path arguments provided for 'preprocess' command.");
            input_path = arg;
        }
    }

    if (input_path.empty()) throw std::runtime_error("A file or folder path argument is required for 'preprocess' command.");
    if (!(convert_flag || validate_source_flag)) throw std::runtime_error("At least one action option (--validate-source, --convert) is required.");
    if (validate_output_flag && !convert_flag) throw std::runtime_error("Option '--validate-output' can only be used with '--convert'.");

    FilePipelineManager pipeline(file_controller_->get_config());

    if (!pipeline.collectFiles(input_path)) {
         throw std::runtime_error("Failed to collect files from the specified path. Aborting.");
    }
    if (validate_source_flag && !pipeline.validateSourceFiles()) {
        throw std::runtime_error("Source file validation failed.");
    }
    if (convert_flag && !pipeline.convertFiles()) {
        throw std::runtime_error("File conversion failed.");
    }
    if (validate_output_flag && !pipeline.validateOutputFiles(day_check_flag)) {
        throw std::runtime_error("Output file validation failed.");
    }
}

// [重構] 處理 'import' 命令 (之前是 --process)
void CliController::handle_database_import() {
    if (args_.size() != 3) throw std::runtime_error("Command 'import' requires exactly one directory path argument.");

    std::cout << YELLOW_COLOR << "Warning:\n" << RESET_COLOR
              << "This command imports pre-processed files. Ensure the path contains only converted files.\n"
              << "Are you sure you want to continue? (y/n): ";
    char confirmation;
    std::cin >> confirmation;
    if (confirmation != 'y' && confirmation != 'Y') {
        std::cout << RED_COLOR << "\nOperation cancelled." << RESET_COLOR << std::endl;
        return;
    }
    std::cout << std::endl;
    action_handler_->run_database_import(args_[2]);
}

// [重構] 處理 'query' 命令
void CliController::handle_query() {
    if (args_.size() < 4) throw std::runtime_error("Command 'query' requires a type and a period argument (e.g., query daily 20240101).");
    
    std::string sub_command = args_[2];
    std::string query_arg = args_[3];
    ReportFormat format = parse_format_option();

    if (sub_command == "daily") {
        std::cout << action_handler_->run_daily_query(query_arg, format);
    } else if (sub_command == "period") {
        std::string token;
        std::istringstream tokenStream(query_arg);
        bool first = true;
        while (std::getline(tokenStream, token, ',')) {
            if (!first) std::cout << "\n" << std::string(40, '-') << "\n";
            try {
                std::cout << action_handler_->run_period_query(std::stoi(token), format);
            } catch (const std::exception&) {
                std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Invalid number '" << token << "' in list. Skipping.\n";
            }
            first = false;
        }
    } else if (sub_command == "monthly") {
        std::cout << action_handler_->run_monthly_query(query_arg, format);
    } else {
        throw std::runtime_error("Unknown query type '" + sub_command + "'.");
    }
}

// [重構] 處理 'export' 命令
void CliController::handle_export() {
    if (args_.size() < 3) throw std::runtime_error("Command 'export' requires a type argument (e.g., export daily).");

    std::string sub_command = args_[2];
    ReportFormat format = parse_format_option();
    
    // 需要参数的子命令
    if (sub_command == "daily" || sub_command == "monthly" || sub_command == "period" || sub_command == "all-period") {
        if (args_.size() < 4) throw std::runtime_error("Argument required for export type '" + sub_command + "'.");
        std::string export_arg = args_[3];

        if (sub_command == "daily") {
            action_handler_->run_export_single_day_report(export_arg, format);
        } else if (sub_command == "monthly") {
            action_handler_->run_export_single_month_report(export_arg, format);
        } else if (sub_command == "period") {
            try {
                action_handler_->run_export_single_period_report(std::stoi(export_arg), format);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid number provided for 'export period': " + export_arg);
            }
        } else if (sub_command == "all-period") {
             std::vector<int> days_list;
             std::string token;
             std::istringstream tokenStream(export_arg);
             while (std::getline(tokenStream, token, ',')) {
                 try {
                     days_list.push_back(std::stoi(token));
                 } catch (const std::exception&) {
                     throw std::runtime_error("Invalid number in days list for 'export all-period': " + token);
                 }
             }
             action_handler_->run_export_all_period_reports_query(days_list, format);
        }
    } 
    // 不需要参数的子命令
    else if (sub_command == "all-daily") {
        action_handler_->run_export_all_daily_reports_query(format);
    } else if (sub_command == "all-monthly") {
        action_handler_->run_export_all_monthly_reports_query(format);
    } else {
        throw std::runtime_error("Unknown export type '" + sub_command + "'.");
    }
}

ReportFormat CliController::parse_format_option() const {
    auto it_f = std::find(args_.begin(), args_.end(), "-f");
    auto it_format = std::find(args_.begin(), args_.end(), "--format");

    std::string format_str;
    if (it_f != args_.end() && std::next(it_f) != args_.end()) {
        format_str = *std::next(it_f);
    } else if (it_format != args_.end() && std::next(it_format) != args_.end()) {
        format_str = *std::next(it_format);
    } else {
        return ReportFormat::Markdown; // 默认格式
    }

    if (format_str == "md" || format_str == "markdown") return ReportFormat::Markdown;
    if (format_str == "tex") return ReportFormat::LaTeX;
    if (format_str == "typ") return ReportFormat::Typ;
     
    throw std::runtime_error("Unsupported format specified: '" + format_str + "'. Supported formats: md, tex, typ.");
}