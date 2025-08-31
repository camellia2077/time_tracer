// time_master_cli/CliController.hpp
#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <string>
#include <vector>
#include <memory>
#include "queries/shared/ReportFormat.hpp"

// 前向声明
class FileController;
class FileProcessingHandler;
class ReportGenerationHandler;

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

    // [核心修改] 更新私有辅助函数以匹配新的命令结构
    void handle_run_pipeline();
    void handle_validate_source();
    void handle_convert();
    void handle_validate_output();
    void handle_database_import();
    void handle_query();
    void handle_export();

    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_HPP