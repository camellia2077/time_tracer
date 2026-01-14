// cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "cli/CliParser.hpp"
// [修复] 更新 AppConfig 路径
#include "common/config/AppConfig.hpp" 

class FileController;
class WorkflowHandler;
class ReportHandler;
class DBManager;

namespace fs = std::filesystem;

class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    void execute();

private:
    CliParser parser_;
    
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

#endif // CLI_CONTROLLER_HPP