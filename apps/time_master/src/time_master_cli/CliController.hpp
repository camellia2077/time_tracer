// time_master_cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "queries/shared/types/ReportFormat.hpp"
#include "CliParser.hpp" // 包含新的解析器头文件

// 前向声明
class FileController;
class FileHandler;
class ReportHandler;

namespace fs = std::filesystem;

class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    void execute();

private:
    CliParser parser_; // 使用新的解析器类替换原始参数

    std::unique_ptr<FileController> file_controller_;
    std::unique_ptr<FileHandler> file_processing_handler_;
    std::unique_ptr<ReportHandler> report_generation_handler_;

    fs::path output_root_path_;
    fs::path exported_files_path_;

    // --- 辅助函数 ---
    void initialize_output_paths();
    void handle_run_pipeline();
    void handle_validate_source();
    void handle_convert();
    void handle_validate_output();
    void handle_database_import();
    void handle_query();
    void handle_export();

    // 解析函数已移至 CliParser 类
};

#endif // CLI_CONTROLLER_HPP