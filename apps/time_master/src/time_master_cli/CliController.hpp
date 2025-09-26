// time_master_cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <map>
#include "time_master_cli/CliParser.hpp"
#include "time_master_cli/commands/ICommand.hpp" // [修改] 引入命令接口

// 前向声明
class FileController;
class FileHandler;
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
    std::unique_ptr<FileHandler> file_processing_handler_;
    std::unique_ptr<ReportHandler> report_generation_handler_;

    // --- [核心修改] 使用 map 来存储和分发命令 ---
    std::map<std::string, std::unique_ptr<ICommand>> commands_;
    
    fs::path output_root_path_;
    fs::path exported_files_path_;

    void initialize_output_paths();
    
    // [移除] 所有 handle_* 私有方法
};

#endif // CLI_CONTROLLER_HPP