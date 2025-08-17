#include "CliController.h"
#include "file_handler/FileController.h"
#include "common/common_utils.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <print>
#include <memory> // 包含 <memory> 以使用 std::make_unique

#include "action_handler/FileProcessingHandler.h"
#include "action_handler/ReportGenerationHandler.h"
// 假设 FileProcessingHandler.h 中定义了 PreprocessingOptions
// 如果没有，可以像这样定义：
/*
struct PreprocessingOptions {
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_check = false;
};
*/


const std::string DATABASE_NAME = "time_data.db";

// [重构] 构造函数现在使用 std::make_unique 初始化智能指针
CliController::CliController(const std::vector<std::string>& args) : args_(args) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = args_[1];

    file_controller_ = std::make_unique<FileController>(args_[0]);
    
    // 实例化处理器，所有权交给 std::unique_ptr
    file_processing_handler_ = std::make_unique<FileProcessingHandler>(
        DATABASE_NAME,
        file_controller_->get_config(),
        file_controller_->get_main_config_path()
    );
    report_generation_handler_ = std::make_unique<ReportGenerationHandler>(
        DATABASE_NAME,
        file_controller_->get_config()
    );
}

// [重构] 析构函数不再需要手动 delete，智能指针会自动处理
CliController::~CliController() = default;

void CliController::execute() {
    // 命令分派结构保持不变
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

void CliController::handle_run_all() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command 'run-all' requires exactly one source directory path argument.");
    }
    file_processing_handler_->run_full_pipeline_and_import(args_[2]);
}

// [重构] handle_preprocess 现在只负责解析参数，并将业务逻辑委托给 FileProcessingHandler
void CliController::handle_preprocess() {
    // 1. 解析参数并填充选项结构体
    PreprocessingOptions options; // 假设 PreprocessingOptions 在 FileProcessingHandler.h 中定义
    std::string input_path;

    for (size_t i = 2; i < args_.size(); ++i) {
        const std::string& arg = args_[i];
        if (arg == "-c" || arg == "--convert") options.convert = true;
        else if (arg == "-vs" || arg == "--validate-source") options.validate_source = true;
        else if (arg == "-vo" || arg == "--validate-output") options.validate_output = true;
        else if (arg == "-edc" || arg == "--enable-day-check") options.enable_day_check = true;
        else if (arg.rfind("-", 0) != 0) {
            if (!input_path.empty()) throw std::runtime_error("Multiple path arguments provided for 'preprocess' command.");
            input_path = arg;
        }
    }

    if (input_path.empty()) throw std::runtime_error("A file or folder path argument is required for 'preprocess' command.");

    if (!options.convert && !options.validate_source && !options.validate_output) {
        throw std::runtime_error("At least one action option (--validate-source, --convert, --validate-output) is required.");
    }

    // 2. 将解析好的参数和路径委托给处理器
    file_processing_handler_->run_preprocessing(input_path, options);
}

void CliController::handle_database_import() {
    if (args_.size() != 3) throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    
    std::println("Now inserting into the database. ");
    std::println("Please ensure the {}data{} has been {}converted and validated{}.",YELLOW_COLOR, RESET_COLOR,YELLOW_COLOR, RESET_COLOR);
    std::println("Are you sure you want to continue? (y/n): ");
    
    char confirmation;
    std::cin >> confirmation;
    if (confirmation != 'y' && confirmation != 'Y') {
        std::cout << RED_COLOR << "\nOperation cancelled." << RESET_COLOR << std::endl;
        return;
    }
    std::cout << std::endl;
    file_processing_handler_->run_database_import(args_[2]);
}

// handle_query 和 handle_export 方法保持不变，因为它们已经很好地委托了任务
void CliController::handle_query() {
    if (args_.size() < 4) throw std::runtime_error("Command 'query' requires a type and a period argument (e.g., query daily 20240101).");
    
    std::string sub_command = args_[2];
    std::string query_arg = args_[3];
    ReportFormat format = parse_format_option();

    if (sub_command == "daily") {
        std::cout << report_generation_handler_->run_daily_query(query_arg, format);
    } else if (sub_command == "period") {
        std::string token;
        std::istringstream tokenStream(query_arg);
        bool first = true;
        while (std::getline(tokenStream, token, ',')) {
            if (!first) std::cout << "\n" << std::string(40, '-') << "\n";
            try {
                std::cout << report_generation_handler_->run_period_query(std::stoi(token), format);
            } catch (const std::exception&) {
                std::cerr << RED_COLOR << "Error: " << RESET_COLOR << "Invalid number '" << token << "' in list. Skipping.\n";
            }
            first = false;
        }
    } else if (sub_command == "monthly") {
        std::cout << report_generation_handler_->run_monthly_query(query_arg, format);
    } else {
        throw std::runtime_error("Unknown query type '" + sub_command + "'.");
    }
}

void CliController::handle_export() {
    if (args_.size() < 3) throw std::runtime_error("Command 'export' requires a type argument (e.g., export daily).");

    std::string sub_command = args_[2];
    ReportFormat format = parse_format_option();
    
    if (sub_command == "daily" || sub_command == "monthly" || sub_command == "period" || sub_command == "all-period") {
        if (args_.size() < 4) throw std::runtime_error("Argument required for export type '" + sub_command + "'.");
        std::string export_arg = args_[3];

        if (sub_command == "daily") {
            report_generation_handler_->run_export_single_day_report(export_arg, format);
        } else if (sub_command == "monthly") {
            report_generation_handler_->run_export_single_month_report(export_arg, format);
        } else if (sub_command == "period") {
            try {
                report_generation_handler_->run_export_single_period_report(std::stoi(export_arg), format);
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
             report_generation_handler_->run_export_all_period_reports_query(days_list, format);
        }
    } 
    else if (sub_command == "all-daily") {
        report_generation_handler_->run_export_all_daily_reports_query(format);
    } else if (sub_command == "all-monthly") {
        report_generation_handler_->run_export_all_monthly_reports_query(format);
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