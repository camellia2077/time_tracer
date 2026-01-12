// cli/CliController.cpp
#include "cli/CliController.hpp"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <filesystem>

#include "config/ConfigLoader.hpp"
#include "io/FileController.hpp"
// common/AppConfig.hpp 已经在头文件中包含了
#include "core/WorkflowHandler.hpp" 
#include "core/ReportHandler.hpp"
#include "core/database/DBManager.hpp"
#include "core/reporting/Exporter.hpp"
#include "core/reporting/ReportGenerator.hpp"
#include "cli/CommandRegistry.hpp"
#include "cli/CliContext.hpp"

namespace fs = std::filesystem;
const std::string DATABASE_FILENAME = "time_data.sqlite3";

CliController::CliController(const std::vector<std::string>& args)
    : parser_(args) {
    
    // 1. 初始化路径变量
    initialize_output_paths();
    
    // 2. 加载配置 [核心修改]
    // 使用 ConfigLoader 加载配置并存储到成员变量 app_config_ 中
    ConfigLoader config_loader(parser_.get_raw_arg(0));
    app_config_ = config_loader.load_configuration(); // <--- 赋值给成员变量

    // 3. 准备 IO 控制器
    file_controller_ = std::make_unique<FileController>();
    file_controller_->prepare_output_directories(output_root_path_, exported_files_path_);

    // 4. 计算数据库路径
    fs::path db_path = output_root_path_ / DATABASE_FILENAME;

    // 5. 创建核心服务，传递成员变量的引用
    // 由于 app_config_ 是 CliController 的成员，它会一直存活直到 CliController 销毁
    workflow_handler_ = std::make_unique<WorkflowHandler>(
        db_path.string(), 
        app_config_, // <--- 传递成员变量
        output_root_path_
    );
    
    db_manager_ = std::make_unique<DBManager>(db_path.string());
    
    const std::string command = parser_.get_command();
    if (command == "query" || command == "export") {
        if (!db_manager_->open_database_if_needed()) {
            throw std::runtime_error("Failed to open database. Please ensure data has been imported first.");
        }
    }
    
    sqlite3* db_connection = db_manager_->get_db_connection();
    // 这里也传递成员变量
    auto report_generator = std::make_unique<ReportGenerator>(db_connection, app_config_);
    auto exporter = std::make_unique<Exporter>(exported_files_path_);
    
    report_generation_handler_ = std::make_unique<ReportHandler>(
        std::move(report_generator), 
        std::move(exporter)
    );
}

CliController::~CliController() = default;

void CliController::execute() {
    const std::string command_name = parser_.get_command();

    CliContext ctx;
    ctx.workflow_handler = workflow_handler_.get(); 
    ctx.report_handler = report_generation_handler_.get();

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
}