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
const std::string DATABASE_FILENAME = "time_data.db";

CliController::CliController(const std::vector<std::string>& args) : args_(args) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = args_[1];

    initialize_output_paths();

    file_controller_ = std::make_unique<FileController>(args_[0]);
    
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

void CliController::initialize_output_paths() {
    if (auto path_opt = parse_output_option()) {
        exported_files_path_ = fs::absolute(*path_opt);
        output_root_path_ = exported_files_path_.parent_path();
    } else {
        fs::path exe_path(args_[0]);
        output_root_path_ = fs::absolute(exe_path.parent_path() / "output");
        exported_files_path_ = output_root_path_ / "exported_files";
    }
    fs::create_directories(output_root_path_);
    fs::create_directories(exported_files_path_);
}

void CliController::execute() {
    if (command_ == "run-pipeline") {
        handle_run_pipeline();
    } else if (command_ == "validate-source") {
        handle_validate_source();
    } else if (command_ == "convert") {
        handle_convert();
    } else if (command_ == "validate-output") {
        handle_validate_output();
    } else if (command_ == "import") {
        handle_database_import();
    } else if (command_ == "query") {
        handle_query();
    } else if (command_ == "export") {
        handle_export();
    } else {
        if (command_ == "preprocess" || command_ == "pre") {
             throw std::runtime_error("The 'preprocess' command is deprecated. Please use 'validate-source', 'convert', or 'validate-output' instead.");
        }
        throw std::runtime_error("Unknown command '" + command_ + "'.");
    }
}

void CliController::handle_run_pipeline() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }
    file_processing_handler_->run_full_pipeline_and_import(args_[2]);
}

void CliController::handle_validate_source() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command 'validate-source' requires exactly one path argument.");
    }
    AppOptions options;
    options.validate_source = true;
    file_processing_handler_->run_preprocessing(args_[2], options);
}

void CliController::handle_convert() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    AppOptions options;
    options.convert = true;
    file_processing_handler_->run_preprocessing(args_[2], options);
}

void CliController::handle_validate_output() {
    if (args_.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;
    
    for (size_t i = 3; i < args_.size(); ++i) {
        if (args_[i] == "-edc" || args_[i] == "--enable-day-check") {
            options.enable_day_count_check = true;
        }
    }
    file_processing_handler_->run_preprocessing(args_[2], options);
}

void CliController::handle_database_import() {
    if (args_.size() != 3) throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    
    std::println("Now inserting into the database.");
    std::println("Please ensure the data has been converted and validated.");
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

std::optional<std::string> CliController::parse_output_option() const {
    auto it_o = std::find(args_.begin(), args_.end(), "-o");
    auto it_output = std::find(args_.begin(), args_.end(), "--output");

    if (it_o != args_.end() && std::next(it_o) != args_.end()) {
        return *std::next(it_o);
    }
    if (it_output != args_.end() && std::next(it_output) != args_.end()) {
        return *std::next(it_output);
    }
    
    return std::nullopt;
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
        return ReportFormat::Markdown;
    }

    if (format_str == "md" || format_str == "markdown") return ReportFormat::Markdown;
    if (format_str == "tex") return ReportFormat::LaTeX;
    if (format_str == "typ") return ReportFormat::Typ;
     
    throw std::runtime_error("Unsupported format specified: '" + format_str + "'. Supported formats: md, tex, typ.");
}