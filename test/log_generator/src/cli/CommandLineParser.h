#pragma once
#include "config/Config.h"
#include "cli/HelpPrinter.h" // [核心修改] 引入 HelpPrinter
#include <optional>
#include <string>
#include <vector>

/**
 * @class CommandLineParser
 * @brief 负责解析和验证从命令行传入的选项。
 */
class CommandLineParser {
public:
    /**
     * @brief 构造函数。
     * @param argc 从 main 函数传入的参数计数。
     * @param argv 从 main 函数传入的参数数组。
     */
    CommandLineParser(int argc, char* argv[]);

    /**
     * @brief 执行解析和验证流程。
     * @return 如果参数有效，则返回一个包含配置的 std::optional<Config>。
     * 如果解析失败或用户请求了帮助/版本，则返回 std::nullopt。
     */
    std::optional<Config> parse();

private:
    // [核心修改] 移除 print_* 方法，用 HelpPrinter 实例代替
    std::vector<std::string> args_;
    HelpPrinter printer_;
};