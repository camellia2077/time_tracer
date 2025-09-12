// time_master_cli/CliController.cpp
#include "CliController.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <print>
#include <memory>
#include <optional>
#include <filesystem>

#include "file_handler/FileController.hpp"
#include "common/common_utils.hpp"
#include "action_handler/FileProcessingHandler.hpp"
#include "action_handler/ReportGenerationHandler.hpp"
#include "common/AppConfig.hpp" // For AppOptions

namespace fs = std::filesystem;
// 数据库的命名以及数据库后缀名
const std::string DATABASE_FILENAME = "time_data.sqlite3";

// 构造函数：初始化 parser_，并使用它来设置其他组件
CliController::CliController(const std::vector<std::string>& args)
    : parser_(args) {
    initialize_output_paths();

    file_controller_ = std::make_unique<FileController>(parser_.get_raw_arg(0));
    
    fs::path db_path = output_root_path_ / DATABASE_FILENAME;
    file_processing_handler_ = std::make_unique<FileProcessingHandler>(
        db_path.string(),
        file_controller_->get_config(),
        file_controller_->get_main_config_path(),
        output_root_path_
    );

    report_generation_handler_ = std::make_unique<ReportGenerationHandler>(
        db_path.string(),
        file_controller_->get_config(),
        exported_files_path_
    );
}

CliController::~CliController() = default;

// 使用 parser_ 获取输出路径选项
void CliController::initialize_output_paths() {
    if (auto path_opt = parser_.get_output_path()) {
        exported_files_path_ = fs::absolute(*path_opt);
        output_root_path_ = exported_files_path_.parent_path();
    } else {
        fs::path exe_path(parser_.get_raw_arg(0));
        output_root_path_ = fs::absolute(exe_path.parent_path() / "output");
        exported_files_path_ = output_root_path_ / "exported_files";
    }
    fs::create_directories(output_root_path_);
    fs::create_directories(exported_files_path_);
}

// 使用 parser_ 获取命令并执行分发
void CliController::execute() {
    const std::string command = parser_.get_command();

    if (command == "run-pipeline") {
        handle_run_pipeline();
    } else if (command == "validate-source") {
        handle_validate_source();
    } else if (command == "convert") {
        handle_convert();
    } else if (command == "validate-output") {
        handle_validate_output();
    } else if (command == "import") {
        handle_database_import();
    } else if (command == "query") {
        handle_query();
    } else if (command == "export") {
        handle_export();
    } else {
        if (command == "preprocess" || command == "pre") {
             throw std::runtime_error("The 'preprocess' command is deprecated. Please use 'validate-source', 'convert', or 'validate-output' instead.");
        }
        throw std::runtime_error("Unknown command '" + command + "'.");
    }
}

// 所有 handle_ 函数都通过 parser_ 获取参数
void CliController::handle_run_pipeline() {
    if (parser_.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }
    file_processing_handler_->run_full_pipeline_and_import(parser_.get_filtered_args()[2]);
}

void CliController::handle_validate_source() {
    if (parser_.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'validate-source' requires exactly one path argument.");
    }
    AppOptions options;
    options.validate_source = true;
    file_processing_handler_->run_preprocessing(parser_.get_filtered_args()[2], options);
}

void CliController::handle_convert() {
    if (parser_.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    AppOptions options;
    options.convert = true;
    file_processing_handler_->run_preprocessing(parser_.get_filtered_args()[2], options);
}

void CliController::handle_validate_output() {
    const auto& filtered_args = parser_.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;
    
    // 检查额外参数，这里可以直接遍历过滤后的参数
    for (size_t i = 3; i < filtered_args.size(); ++i) {
        if (filtered_args[i] == "-edc" || filtered_args[i] == "--enable-day-check") {
            options.enable_day_count_check = true;
        }
    }
    file_processing_handler_->run_preprocessing(filtered_args[2], options);
}

void CliController::handle_database_import() {
    if (parser_.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    }
    
    std::println("Now inserting into the database.");
    std::println("Please ensure the data has been converted and validated.");
    std::println("Are you sure you want to continue? (y/n): ");
    
    char confirmation;
    std::cin >> confirmation;
    if (confirmation != 'y' && confirmation != 'Y') {
        std::cout << "\033[31m\nOperation cancelled.\033[0m" << std::endl;
        return;
    }
    std::cout << std::endl;
    file_processing_handler_->run_database_import(parser_.get_filtered_args()[2]);
}

void CliController::handle_query() {
    const auto& filtered_args = parser_.get_filtered_args();
    if (filtered_args.size() < 4) {
        throw std::runtime_error("Command 'query' requires a type and a period argument (e.g., query daily 20240101).");
    }
    
    std::string sub_command = filtered_args[2];
    std::string query_arg = filtered_args[3];
    ReportFormat format = parser_.get_report_format();

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
                std::cerr << "\033[31mError: \033[0mInvalid number '" << token << "' in list. Skipping.\n";
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
    const auto& filtered_args = parser_.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'export' requires a type argument (e.g., export daily).");
    }

    std::string sub_command = filtered_args[2];
    ReportFormat format = parser_.get_report_format();

    if (sub_command == "daily" || sub_command == "monthly" || sub_command == "period" || sub_command == "all-period") {
        if (filtered_args.size() < 4) {
            throw std::runtime_error("Argument required for export type '" + sub_command + "'.");
        }
        std::string export_arg = filtered_args[3];

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