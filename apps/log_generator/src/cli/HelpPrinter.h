#pragma once
#include <string>

/**
 * @class HelpPrinter
 * @brief 负责向控制台打印所有用户帮助信息。
 *
 * 这个类将所有UI输出逻辑（如用法、版本、错误消息）与
 * 命令行解析的核心逻辑解耦。
 */
class HelpPrinter {
public:
    /**
     * @brief 构造函数。
     * @param prog_name 程序的名称 (argv[0])，用于显示在用法信息中。
     */
    explicit HelpPrinter(const std::string& prog_name);

    /**
     * @brief 打印完整的用法/帮助信息。
     */
    void print_usage() const;

    /**
     * @brief 打印版本信息。
     */
    void print_version() const;

    /**
     * @brief 打印一条格式化的错误消息。
     * @param message 要显示的核心错误信息。
     */
    void print_error(const std::string& message) const;

private:
    std::string prog_name_;
};