// time_master_cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <filesystem>
#include "queries/shared/ReportFormat.hpp"

// 前向声明
class FileController;
class FileProcessingHandler;
class ReportGenerationHandler;

namespace fs = std::filesystem;

class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    void execute();

private:
    std::vector<std::string> args_;
    std::string command_;
    
    std::unique_ptr<FileController> file_controller_;
    std::unique_ptr<FileProcessingHandler> file_processing_handler_;
    std::unique_ptr<ReportGenerationHandler> report_generation_handler_;

    fs::path output_root_path_;      // 新增：主输出目录 (e.g., ./output)
    fs::path exported_files_path_; // 新增：报告专用目录 (e.g., ./output/exported_files)

    // --- 辅助函数 ---
    void initialize_output_paths(); // 修改：初始化所有输出路径的函数
    void handle_run_pipeline();
    void handle_validate_source();
    void handle_convert();
    void handle_validate_output();
    void handle_database_import();
    void handle_query();
    void handle_export();

    ReportFormat parse_format_option() const;
    std::optional<std::string> parse_output_option() const;
};

#endif // CLI_CONTROLLER_HPP