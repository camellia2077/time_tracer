// cli/impl/app/cli_application.hpp
#ifndef CLI_IMPL_APP_CLI_APPLICATION_HPP_
#define CLI_IMPL_APP_CLI_APPLICATION_HPP_

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "cli/framework/core/command_parser.hpp"
#include "common/config/app_config.hpp" 

class FileController;
class WorkflowHandler;
class ReportHandler;
class DBManager;

namespace fs = std::filesystem;

class CliApplication {
public:
    explicit CliApplication(const std::vector<std::string>& args);
    ~CliApplication();

    void execute();

private:
    CommandParser parser_;
    
    // [新增] 将 AppConfig 提升为成员变量，确保其生命周期
    AppConfig app_config_;

    // --- 依赖的服务 ---
    std::unique_ptr<FileController> file_controller_;
    std::unique_ptr<DBManager> db_manager_;
    std::unique_ptr<WorkflowHandler> workflow_handler_; 
    std::unique_ptr<ReportHandler> report_generation_handler_;

    fs::path output_root_path_;
    fs::path exported_files_path_;

    void initialize_output_paths();
};

#endif // CLI_IMPL_APP_CLI_APPLICATION_HPP_