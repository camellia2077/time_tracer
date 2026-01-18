// cli/framework/core/command_parser.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_
#define CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_

#include <string>
#include <vector>
#include <optional>

class CommandParser {
public:
    explicit CommandParser(const std::vector<std::string>& args);

    std::string get_command() const;
    const std::vector<std::string>& get_filtered_args() const;
    std::string get_raw_arg(size_t index) const;

    // [通用接口] 获取带值的选项 (如 -o path)
    // keys 示例: {"-o", "--output"}
    std::optional<std::string> get_option(const std::vector<std::string>& keys) const;

    // [通用接口] 检查布尔开关 (如 --no-save)
    bool has_flag(const std::vector<std::string>& keys) const;

private:
    std::vector<std::string> raw_args_;
    std::string command_;
    std::vector<std::string> filtered_args_;

    void parse();
    static std::vector<std::string> filter_global_options(const std::vector<std::string>& original_args);
};

#endif // CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_