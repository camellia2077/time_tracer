// cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "cli/CliParser.hpp"

class FileController;
class WorkflowHandler; // [修改] 类名变更 FileHandler -> WorkflowHandler
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

    // --- 依赖的服务 ---
    std::unique_ptr<FileController> file_controller_;
    std::unique_ptr<DBManager> db_manager_;
    
    // [修改] 类型和变量名变更
    std::unique_ptr<WorkflowHandler> workflow_handler_; 
    std::unique_ptr<ReportHandler> report_generation_handler_;

    fs::path output_root_path_;
    fs::path exported_files_path_;

    void initialize_output_paths();
};

#endif // CLI_CONTROLLER_HPP