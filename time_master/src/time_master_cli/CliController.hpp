// time_master_cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory> // 包含 <memory> 以使用 std::unique_ptr
#include "queries/shared/ReportFormat.hpp" // 结构定义在这

// 前向声明
class FileController;
class FileProcessingHandler;
class ReportGenerationHandler;

/**
 * @class CliController
 * @brief 处理所有命令行界面 (CLI) 逻辑。
 *
 * 该类解析命令行参数，并根据参数调用相应的处理器
 * (FileProcessingHandler 或 ReportGenerationHandler)。
 */
class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController(); // 析构函数仍然需要，但会是默认的

    void execute();

private:
    std::vector<std::string> args_;
    std::string command_;
    
    // 使用智能指针管理对象的生命周期
    std::unique_ptr<FileController> file_controller_;
    std::unique_ptr<FileProcessingHandler> file_processing_handler_;
    std::unique_ptr<ReportGenerationHandler> report_generation_handler_;

    // --- 私有辅助函数，用于处理命令分支 ---
    void handle_run_all();
    void handle_preprocess();
    void handle_database_import();
    void handle_query();
    void handle_export();

    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_HPP