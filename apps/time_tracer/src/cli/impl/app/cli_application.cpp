// cli/impl/app/cli_application.cpp
#include "cli/impl/app/cli_application.hpp"
#include "cli/impl/utils/help_formatter.hpp"

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

namespace fs = std::filesystem;
const std::string DATABASE_FILENAME = "time_data.sqlite3";

// [新增] 辅助函数，定义应用特定的全局参数规则
static ParserConfig get_app_parser_config() {
    return {
        // global_value_options: 这些选项后面跟着一个值，需要从位置参数中剔除
        {
            "-o", "--output", 
            "-f", "--format", 
            "--date-check", 
            "--db", "--database"
        },
        // global_flag_options: 这些是布尔开关
        {
            "--save-processed", 
            "--no-save", 
            "--no-date-check"
        }
    };
}
// [修改] 在初始化列表中调用 get_app_parser_config()
CliApplication::CliApplication(const std::vector<std::string>& args)
    : parser_(args, get_app_parser_config()) {
    
    // 0. 初始化服务容器
    app_context_ = std::make_shared<AppContext>();

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

    // 2. 初始化基础设施
    file_controller_ = std::make_unique<FileController>();
    file_controller_->prepare_output_directories(output_root_path_, exported_files_path_);
    
    db_manager_ = std::make_unique<DBManager>(db_path.string());

    // 3. 初始化并注入核心服务到 Context
    // [WorkflowHandler]
    auto workflow_impl = std::make_shared<WorkflowHandler>(
        db_path.string(), 
        app_config_, 
        output_root_path_
    );
    app_context_->workflow_handler = workflow_impl; // 注入接口

    // 数据库连接检查 (逻辑保持不变)
    const std::string command = parser_.get_command();
    if (command == "query" || command == "export") {
        if (!db_manager_->open_database_if_needed()) {
            throw std::runtime_error("Failed to open database at: " + db_path.string() + "\nPlease ensure data has been imported or check the path.");
        }
    }
    sqlite3* db_connection = db_manager_->get_db_connection();
    
    // [ReportHandler]
    auto report_generator = std::make_unique<ReportGenerator>(db_connection, app_config_);
    auto exporter = std::make_unique<Exporter>(exported_files_path_);
    
    auto report_impl = std::make_shared<ReportHandler>(
        std::move(report_generator), 
        std::move(exporter)
    );
    app_context_->report_handler = report_impl; // 注入接口
}

CliApplication::~CliApplication() = default;

void CliApplication::execute() {
    // [新增] 处理 Help 全局标记
    // 检查是否仅请求帮助，或者没有提供命令
    bool request_help = parser_.has_flag({"--help", "-h"});
    std::string command_name;
    try {
        command_name = parser_.get_command();
    } catch (...) {
        // 如果无法解析命令（例如只有 ./timetracer），默认为 help
        request_help = true;
    }

    if (request_help) {
        // 使用 Registry 批量创建所有命令以提取其元数据
        // 注意：这里我们利用已经初始化的 app_context_
        auto commands = CommandRegistry<AppContext>::Instance().CreateAllCommands(*app_context_);
        print_full_usage(parser_.get_raw_arg(0).c_str(), commands);
        return;
    }

    // [原有逻辑] 执行具体命令
    // 注意：这里不再需要手动检查 app_context_ 的完整性，因为我们在构造函数里已经保证了
    auto command = CommandRegistry<AppContext>::Instance().CreateCommand(command_name, *app_context_);

    if (command) {
        try {
            command->execute(parser_);
        } catch (const std::exception& e) {
            std::cerr << RED_COLOR << "Error executing command '" << command_name << "': " << e.what() << RESET_COLOR << std::endl;
            // 可选：出错时打印该特定命令的帮助
            // print_command_usage(command_name, *command); 
        }
    } else {
        std::cerr << RED_COLOR << "Unknown command '" << command_name << "'." << RESET_COLOR << std::endl;
        std::cout << "Run with --help to see available commands." << std::endl;
    }
}

void CliApplication::initialize_output_paths() {}