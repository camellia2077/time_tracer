#pragma once
#include "config/Config.h" // 需要用到Config结构体
#include <optional>
#include <string>

/**
 * @class CommandLineParser
 * @brief 负责解析和验证从命令行传入的参数。
 *
 * 这个类封装了所有与命令行输入（argc, argv）相关的逻辑，
 * 包括参数解析、有效性验证以及打印用法/版本信息。
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
     * @return 如果参数有效，则返回一个包含配置的 std::optional<Config>；
     * 如果参数无效或用户请求帮助/版本信息，则返回 std::nullopt。
     */
    std::optional<Config> parse();

    /**
     * @brief 检查用户是否请求了版本信息 (例如, --version)。
     * @return 如果是，则返回 true。
     */
    bool version_requested() const;
    
    /**
     * @brief [核心修改] 将 print_version 移至 public 区域，以便 main.cpp 可以调用。
     * 打印程序的版本信息到标准输出流。
     */
    void print_version() const;

    /**
     * @brief [核心修改] 将 print_usage 移至 public 区域，以便 main.cpp 可以调用。
     * 打印程序的用法说明到标准错误流。
     */
    void print_usage() const;

private:
    int argc_;
    char** argv_;
    std::string prog_name_;
};