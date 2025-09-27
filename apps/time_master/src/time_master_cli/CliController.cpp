// time_master_cli/CliController.cpp
#include "CliController.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>

// 引入所有需要的服务头文件
#include "file_handler/FileController.hpp"
#include "common/AppConfig.hpp"
#include "action_handler/FileHandler.hpp"
#include "action_handler/ReportHandler.hpp"
#include "action_handler/database/DBManager.hpp"
#include "action_handler/query/QueryManager.hpp"
#include "action_handler/reporting/Exporter.hpp"
#include "action_handler/reporting/ReportGenerator.hpp" // [ADDED] 引入 ReportGenerator

// 引入所有具体的命令类
#include "time_master_cli/commands/query/Query.hpp"
#include "time_master_cli/commands/export/Export.hpp"
#include "time_master_cli/commands/pipeline/Run.hpp"
#include "time_master_cli/commands/pipeline/ValidateSource.hpp"
#include "time_master_cli/commands/pipeline/Convert.hpp"
#include "time_master_cli/commands/pipeline/ValidateOutput.hpp"
#include "time_master_cli/commands/pipeline/Import.hpp"


namespace fs = std::filesystem;
const std::string DATABASE_FILENAME = "time_data.sqlite3";

CliController::CliController(const std::vector<std::string>& args)
    : parser_(args) {
    // 1. 初始化路径和配置加载器
    initialize_output_paths();
    file_controller_ = std::make_unique<FileController>(parser_.get_raw_arg(0));
    const AppConfig& config = file_controller_->get_config();
    fs::path db_path = output_root_path_ / DATABASE_FILENAME;

    // 2. 创建核心服务
    file_processing_handler_ = std::make_unique<FileHandler>(
        db_path.string(), config, file_controller_->get_main_config_path(), output_root_path_
    );
    
    db_manager_ = std::make_unique<DBManager>(db_path.string());
    
    // 在将数据库连接传递出去之前，必须先尝试打开它
    const std::string command = parser_.get_command();
    if (command == "query" || command == "export") {
        if (!db_manager_->open_database_if_needed()) {
            throw std::runtime_error("Failed to open database. Please ensure data has been imported first.");
        }
    }
    
    // --- [ 核心修复 ] ---
    // 根据重构后的类，正确地实例化服务
    
    // a. 获取数据库连接
    sqlite3* db_connection = db_manager_->get_db_connection();

    // b. 创建 QueryManager
    auto query_manager = std::make_unique<QueryManager>(db_connection, config);

    // c. 创建新的 ReportGenerator
    auto report_generator = std::make_unique<ReportGenerator>(db_connection, config);

    // d. 创建 Exporter (现在只需要导出路径)
    auto exporter = std::make_unique<Exporter>(exported_files_path_);
    
    // e. 创建 ReportHandler (现在需要三个依赖项)
    report_generation_handler_ = std::make_unique<ReportHandler>(
        std::move(query_manager), 
        std::move(report_generator), 
        std::move(exporter)
    );

    // 3. 注册所有命令
    commands_["run-pipeline"] = std::make_unique<Run>(*file_processing_handler_);
    commands_["validate-source"] = std::make_unique<ValidateSource>(*file_processing_handler_);
    commands_["convert"] = std::make_unique<Convert>(*file_processing_handler_);
    commands_["validate-output"] = std::make_unique<ValidateOutput>(*file_processing_handler_);
    commands_["import"] = std::make_unique<Import>(*file_processing_handler_);
    
    commands_["query"] = std::make_unique<Query>(*report_generation_handler_);
    commands_["export"] = std::make_unique<Export>(*report_generation_handler_);
}

CliController::~CliController() = default;

void CliController::execute() {
    const std::string command_name = parser_.get_command();

    auto it = commands_.find(command_name);
    if (it != commands_.end()) {
        it->second->execute(parser_);
    } else {
        if (command_name == "preprocess" || command_name == "pre") {
             throw std::runtime_error("The 'preprocess' command is deprecated. Please use 'validate-source', 'convert', or 'validate-output' instead.");
        }
        throw std::runtime_error("Unknown command '" + command_name + "'.");
    }
}

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