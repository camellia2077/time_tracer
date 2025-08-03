// cli/CliController.h
#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include <string>
#include <vector>
#include "queries/shared/ReportFormat.h"

// 前向声明
class ActionHandler;
class FileController;

/**
 * @class CliController
 * @brief 处理所有命令行接口 (CLI) 的逻辑。
 *
 * 此类解析命令行参数，并根据这些参数调用相应的 ActionHandler 方法。
 * 它是 CLI 的主要业务逻辑中心。
 */
class CliController {
public:
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    /**
     * @brief 执行由命令行参数指定的命令。
     *
     * 这是该类的主要入口点。它将解析命令并分派给适当的处理函数。
     * 失败时抛出 std::runtime_error。
     */
    void execute();

private:
    std::vector<std::string> args_;
    std::string command_;
    ActionHandler* action_handler_;
    FileController* file_controller_;

    // --- 用于处理命令分支的私有辅助函数 ---
    void handle_full_pipeline();
    void handle_manual_preprocessing();
    void handle_database_import();
    void handle_query();
    void handle_export();

    /**
     * @brief 从命令行解析格式选项 (-f, --format)。
     * @return 一个 ReportFormat 枚举值。如果未指定，则默认为 Markdown。
     */
    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_H