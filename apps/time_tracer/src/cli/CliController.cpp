// cli/CliController.cpp
#include "cli/CliController.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>

// 引入服务头文件
#include "file_handler/FileController.hpp"
#include "common/AppConfig.hpp"
#include "action_handler/WorkflowHandler.hpp" // [确认] 引入 WorkflowHandler
#include "action_handler/ReportHandler.hpp"
#include "action_handler/database/DBManager.hpp"
#include "action_handler/reporting/Exporter.hpp"
#include "action_handler/reporting/ReportGenerator.hpp"

// 引入注册中心和上下文
#include "cli/CommandRegistry.hpp"
#include "cli/CliContext.hpp"

namespace fs = std::filesystem;
const std::string DATABASE_FILENAME = "time_data.sqlite3";

CliController::CliController(const std::vector<std::string>& args)
    : parser_(args) {
    // 1. 初始化路径和配置
    initialize_output_paths();
    file_controller_ = std::make_unique<FileController>(parser_.get_raw_arg(0));
    const AppConfig& config = file_controller_->get_config();
    fs::path db_path = output_root_path_ / DATABASE_FILENAME;

    // 2. 创建核心服务
    // [修改] 使用 WorkflowHandler，并移除 main_config_path 参数
    workflow_handler_ = std::make_unique<WorkflowHandler>(
        db_path.string(), 
        config, 
        // file_controller_->get_main_config_path(), // [移除] 该参数已在 WorkflowHandler 中删除
        output_root_path_
    );
    
    db_manager_ = std::make_unique<DBManager>(db_path.string());
    
    // 打开数据库连接逻辑
    const std::string command = parser_.get_command();
    if (command == "query" || command == "export") {
        if (!db_manager_->open_database_if_needed()) {
            throw std::runtime_error("Failed to open database. Please ensure data has been imported first.");
        }
    }
    
    // 创建报告相关服务
    sqlite3* db_connection = db_manager_->get_db_connection();
    auto report_generator = std::make_unique<ReportGenerator>(db_connection, config);
    auto exporter = std::make_unique<Exporter>(exported_files_path_);
    
    report_generation_handler_ = std::make_unique<ReportHandler>(
        std::move(report_generator), 
        std::move(exporter)
    );
}

CliController::~CliController() = default;

void CliController::execute() {
    const std::string command_name = parser_.get_command();

    // 1. 组装上下文
    CliContext ctx;
    // [修改] 这里假设 CliContext 的成员变量也相应重命名为了 workflow_handler
    // 如果 CliContext 还没改，这里需要保持 ctx.file_handler = workflow_handler_.get();
    ctx.workflow_handler = workflow_handler_.get(); 
    ctx.report_handler = report_generation_handler_.get();

    // 2. 从注册中心获取命令
    auto command = CommandRegistry::instance().create_command(command_name, ctx);

    if (command) {
        command->execute(parser_);
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