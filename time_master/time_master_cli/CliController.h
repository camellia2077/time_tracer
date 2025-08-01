// cli/CliController.h
#include "common/pch.h"
#ifndef CLI_CONTROLLER_H
#define CLI_CONTROLLER_H

#include <string>
#include <vector>
#include "queries/shared/ReportFormat.h"

// 前向声明以避免在头文件中包含重量级依赖
class ActionHandler;
class FileController;

/**
 * @class CliController
 * @brief 处理所有命令行界面 (CLI) 的逻辑。
 * 
 * 此类解析命令行参数，并根据这些参数调用适当的 ActionHandler 方法。
 * 它是 CLI 的主要业务逻辑中心。
 */
class CliController {
public:
    /**
     * @brief 构造一个 CliController 实例。
     * @param args 来自 main 函数的命令行参数列表。
     */
    explicit CliController(const std::vector<std::string>& args);
    ~CliController();

    /**
     * @brief 执行由命令行参数指定的命令。
     * 
     * 这是该类的主要入口点。它会解析命令并分派到相应的处理函数。
     * 如果发生错误，它会抛出 std::runtime_error。
     */
    void execute();

private:
    std::vector<std::string> args_; // 存储命令行参数
    std::string command_;           // 存储主命令 (e.g., "-q", "-a")
    ActionHandler* action_handler_; // 用于执行核心业务逻辑的处理器
    FileController* file_controller_; // 用于管理配置文件

    // --- 私有辅助函数，用于处理不同的命令分支 ---
    void handle_full_pipeline();
    void handle_manual_preprocessing();
    void handle_database_import();
    void handle_query();
    void handle_export();

    /**
     * @brief 解析命令行中的格式选项 (-f, --format)。
     * @return ReportFormat 枚举值。如果未指定，默认为 Markdown。
     */
    ReportFormat parse_format_option() const;
};

#endif // CLI_CONTROLLER_H