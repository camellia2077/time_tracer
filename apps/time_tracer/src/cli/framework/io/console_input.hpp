// cli/framework/io/console_input.hpp
#ifndef CLI_FRAMEWORK_IO_CONSOLE_INPUT_HPP_
#define CLI_FRAMEWORK_IO_CONSOLE_INPUT_HPP_

#include <string>
#include <vector>

/**
 * @class ConsoleInput
 * @brief 负责处理 CLI 层面的用户输入交互。
 */
class ConsoleInput {
public:
    /**
     * @brief 提示用户输入文件路径（支持多个，空格分隔）。
     * @details 逻辑迁移自 db_inserter/DataImporter.cpp 中的 UserInteractor。
     * @param message 提示信息。
     * @return 用户输入的路径列表。
     */
    std::vector<std::string> prompt_for_paths(const std::string& message = "Enter .json file(s) or directory path(s) to process: ") const;

    /**
     * @brief 提示用户进行确认 (y/n)。
     * @param message 提示信息。
     * @return 如果用户输入 y 或 Y 返回 true，否则返回 false。
     */
    bool prompt_confirmation(const std::string& message = "Are you sure you want to continue?") const;
};

#endif // CLI_FRAMEWORK_IO_CONSOLE_INPUT_HPP_