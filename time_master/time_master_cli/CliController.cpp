#include "common/pch.h"
#include "CliController.h"
#include "action_handler/ActionHandler.h"
#include "file_handler/FileController.h"
#include "common/common_utils.h"
#include "action_handler/ExportUtils.h" // Needed for get_report_format_details

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm> // for std::find
#include <functional> // for std::function
#include <filesystem> // for std::filesystem
#include <fstream>    // for std::ofstream


const std::string DATABASE_NAME = "time_data.db";

CliController::CliController(const std::vector<std::string>& args) : args_(args) {
    if (args.size() < 2) {
        throw std::runtime_error("No command provided.");
    }
    command_ = args_[1];

    // Initialize core components
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
    if (command_ == "-a" || command_ == "--all") {
        handle_full_pipeline();
    } else if (command_ == "-c" || command_ == "--convert" || command_ == "-vs" || command_ == "--validate-source" || command_ == "-vo" || command_ == "--validate-output" || command_ == "-edc" || command_ == "--enable-day-check") {
        handle_manual_preprocessing();
    } else if (command_ == "-p" || command_ == "--process") {
        handle_database_import();
    } else if (command_ == "-q" || command_ == "--query") {
        handle_query();
    } else if (command_ == "-e" || command_ == "--export") {
        handle_export();
    } else {
        throw std::runtime_error("Unknown command '" + command_ + "'. Use --help for usage information.");
    }
}

void CliController::handle_full_pipeline() {
    if (args_.size() != 3) {
        throw std::runtime_error("Command '" + command_ + "' requires exactly one source directory path.");
    }
    action_handler_->run_full_pipeline_and_import(args_[2]);
}

void CliController::handle_manual_preprocessing() {
    bool convert_flag = false, validate_source_flag = false, validate_output_flag = false, day_check_flag = false, path_provided = false;
    std::string input_path;

    for (size_t i = 1; i < args_.size(); ++i) {
        const std::string& arg = args_[i];
        if (arg == "-c" || arg == "--convert") convert_flag = true;
        else if (arg == "-vs" || arg == "--validate-source") validate_source_flag = true;
        else if (arg == "-vo" || arg == "--validate-output") validate_output_flag = true;
        else if (arg == "-edc" || arg == "--enable-day-check") day_check_flag = true;
        else if (arg.rfind("-", 0) != 0) {
            if (path_provided) throw std::runtime_error("Multiple paths provided for file processing.");
            input_path = arg;
            path_provided = true;
        }
    }

    if (!path_provided) throw std::runtime_error("A file or folder path is required for manual pre-processing commands.");
    if (validate_output_flag && !convert_flag) throw std::runtime_error("The --validate-output (-vo) flag can only be used with the --convert (-c) flag.");

    if (!action_handler_->collectFiles(input_path)) {
         throw std::runtime_error("Failed to collect files from the specified path. Aborting.");
    }
    if (validate_source_flag && !action_handler_->validateSourceFiles()) throw std::runtime_error("Source file validation failed.");
    if (convert_flag && !action_handler_->convertFiles()) throw std::runtime_error("File conversion failed.");
    if (validate_output_flag && !action_handler_->validateOutputFiles(day_check_flag)) throw std::runtime_error("Output file validation failed.");
}

void CliController::handle_database_import() {
    if (args_.size() != 3) throw std::runtime_error("Command '" + command_ + "' requires exactly one directory path.");

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

void CliController::handle_query() {
    if (args_.size() < 4) throw std::runtime_error("Query command requires a sub-command and an argument (e.g., -q d 20240101).");
    
    std::string sub_command = args_[2];
    std::string query_arg = args_[3];
    ReportFormat format = parse_format_option();

    if (sub_command == "d" || sub_command == "daily") {
        std::cout << action_handler_->run_daily_query(query_arg, format);
    } else if (sub_command == "p" || sub_command == "period") {
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
    } else if (sub_command == "m" || sub_command == "monthly") {
        std::cout << action_handler_->run_monthly_query(query_arg, format);
    } else {
        throw std::runtime_error("Unknown query sub-command '" + sub_command + "'.");
    }
}

void CliController::handle_export() {
    if (args_.size() < 3) throw std::runtime_error("Export command requires a sub-command (e.g., --export daily).");

    std::string sub_command = args_[2];
    ReportFormat format = parse_format_option();

    if (sub_command == "daily") {
        if (args_.size() < 4) throw std::runtime_error("Command '--export daily' requires a date (YYYYMMDD).");
        std::string date = args_[3];
        export_single_report("日报", date, [&]() {
            return action_handler_->run_daily_query(date, format);
        }, format);

    } else if (sub_command == "monthly") {
        if (args_.size() < 4) throw std::runtime_error("Command '--export monthly' requires a month (YYYYMM).");
        std::string month = args_[3];
        export_single_report("月报", month, [&]() {
            return action_handler_->run_monthly_query(month, format);
        }, format);

    } else if (sub_command == "period") {
        if (args_.size() < 4) throw std::runtime_error("Command '--export period' requires a list of days (e.g., 7 or 7,30,90).");
        
        std::vector<int> days_list;
        std::string token;
        std::istringstream tokenStream(args_[3]);
        while (std::getline(tokenStream, token, ',')) {
            try {
                days_list.push_back(std::stoi(token));
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid number provided in the days list: " + token);
            }
        }
        action_handler_->run_export_all_period_reports_query(days_list, format);

    } else if (sub_command == "all-daily") {
        action_handler_->run_export_all_daily_reports_query(format);

    } else if (sub_command == "all-monthly") {
        action_handler_->run_export_all_monthly_reports_query(format);

    } else {
        throw std::runtime_error("Unknown export sub-command '" + sub_command + "'. Use 'daily', 'monthly', 'period', 'all-daily', or 'all-monthly'.");
    }
}

void CliController::export_single_report(
    const std::string& report_type,
    const std::string& identifier,
    std::function<std::string()> content_generator,
    ReportFormat format
) const {
    auto format_details_opt = ExportUtils::get_report_format_details(format);
    if (!format_details_opt) {
        return;
    }
    const auto& format_details = *format_details_opt;

    std::string report_content = content_generator();
    if (report_content.empty() || report_content.find("No time records") != std::string::npos) {
        std::cout << YELLOW_COLOR << "信息: 没有为 " << identifier << " 找到可导出的" << report_type << "内容。" << RESET_COLOR << std::endl;
        return;
    }

    std::filesystem::path export_dir = "exported_files";
    std::filesystem::path output_path = export_dir / (identifier + "_report" + format_details.extension);
    
    std::filesystem::create_directories(export_dir);

    std::ofstream output_file(output_path);
    if (!output_file) {
        std::cerr << RED_COLOR << "错误: 无法创建或打开文件: " << output_path << RESET_COLOR << std::endl;
        return;
    }
    output_file << report_content;
    
    std::cout << GREEN_COLOR << "成功: " << report_type << "已成功导出到 " << std::filesystem::absolute(output_path) << RESET_COLOR << std::endl;
}

ReportFormat CliController::parse_format_option() const {
    auto it_f = std::find(args_.begin(), args_.end(), "-f");
    auto it_format = std::find(args_.begin(), args_.end(), "--format");

    std::string format_str = "md";
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
     
    throw std::runtime_error("Unsupported format specified: '" + format_str + "'. Supported formats: md, markdown, tex, typ.");
}