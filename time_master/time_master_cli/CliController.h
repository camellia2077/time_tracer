// time_master_cli/CliController.h
#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include <string>
#include <vector>
#include "queries/shared/ReportFormat.h"

// 前向声明
class FileController;
class FileProcessingHandler;
class ReportGenerationHandler;

/**
 * @class CliController
 * @brief 处理所有命令行界面 (CLI) 逻辑 (已重构)。
 *
 * 该类解析命令行参数，并根据参数调用相应的处理器
 * (FileProcessingHandler 或 ReportGenerationHandler)。
 */
class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    void execute();

private:
    std::vector<std::string> args_;
    std::string command_;
    
    // 成员变量已更新为新的、更专注的处理器
    FileController* file_controller_;
    FileProcessingHandler* file_processing_handler_;
    ReportGenerationHandler* report_generation_handler_;

    // --- 私有辅助函数，用于处理命令分支 ---
    void handle_run_all();
    void handle_preprocess();
    void handle_database_import();
    void handle_query();
    void handle_export();

    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_H