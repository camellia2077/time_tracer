// cli/impl/app/cli_application.cpp
#include "cli/impl/app/cli_application.hpp"

#include "cli/framework/interfaces/i_command.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>

#include "cli/framework/core/command_registry.hpp"
#include "config/config_loader.hpp"
#include "io/file_controller.hpp"
#include "core/workflow_handler.hpp" 
#include "core/reporting/report_handler.hpp"
#include "core/database/db_manager.hpp"
#include "core/reporting/export/exporter.hpp"
#include "core/reporting/generator/report_generator.hpp"
#include "cli/impl/app/app_context.hpp" // 为模板参数

namespace fs = std::filesystem;
const std::string DATABASE_FILENAME = "time_data.sqlite3";

CliApplication::CliApplication(const std::vector<std::string>& args)
    : parser_(args) {
    
    // 1. 路径初始化
    fs::path exe_path(parser_.get_raw_arg(0));
    fs::path default_output_root = fs::absolute(exe_path.parent_path() / "output");

    fs::path db_path;
    if (auto user_db_path = parser_.get_option({"--db", "--database"})) {
        db_path = fs::absolute(*user_db_path);
    } else {
        db_path = default_output_root / DATABASE_FILENAME;
    }

    if (auto path_opt = parser_.get_option({"-o", "--output"})) {
        exported_files_path_ = fs::absolute(*path_opt);
    } else {
        exported_files_path_ = default_output_root / "exported_files";
    }
    output_root_path_ = default_output_root;

    ConfigLoader config_loader(parser_.get_raw_arg(0));
    app_config_ = config_loader.load_configuration(); 

    file_controller_ = std::make_unique<FileController>();
    file_controller_->prepare_output_directories(output_root_path_, exported_files_path_);

    workflow_handler_ = std::make_unique<WorkflowHandler>(
        db_path.string(), 
        app_config_, 
        output_root_path_
    );
    
    db_manager_ = std::make_unique<DBManager>(db_path.string());
    
    const std::string command = parser_.get_command();
    if (command == "query" || command == "export") {
        if (!db_manager_->open_database_if_needed()) {
            throw std::runtime_error("Failed to open database at: " + db_path.string() + "\nPlease ensure data has been imported or check the path.");
        }
    }
    
    sqlite3* db_connection = db_manager_->get_db_connection();
    
    auto report_generator = std::make_unique<ReportGenerator>(db_connection, app_config_);
    auto exporter = std::make_unique<Exporter>(exported_files_path_);
    
    report_generation_handler_ = std::make_unique<ReportHandler>(
        std::move(report_generator), 
        std::move(exporter)
    );
}

CliApplication::~CliApplication() = default;

void CliApplication::execute() {
    const std::string command_name = parser_.get_command();

    AppContext ctx;
    ctx.workflow_handler = workflow_handler_.get(); 
    ctx.report_handler = report_generation_handler_.get();

    // [修改] 显式指定模板参数 <AppContext>
    auto command = CommandRegistry<AppContext>::Instance().CreateCommand(command_name, ctx);

    if (command) {
        command->execute(parser_);
    } else {
        if (command_name == "preprocess" || command_name == "pre") {
             throw std::runtime_error("The 'preprocess' command is deprecated. Please use 'validate-source', 'convert', or 'validate-output' instead.");
        }
        throw std::runtime_error("Unknown command '" + command_name + "'.");
    }
}

void CliApplication::initialize_output_paths() {}