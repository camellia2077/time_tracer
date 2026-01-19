// cli/framework/core/command_parser.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_
#define CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_

#include <string>
#include <vector>
#include <optional>

/**
 * @brief 解析器配置，用于定义哪些选项是全局的，需要在过滤阶段剔除或特殊处理。
 */
struct ParserConfig {
    // 带值的选项（例如：-o, --db），解析器会跳过它们及其紧随的参数
    std::vector<std::string> global_value_options;
    // 布尔开关（例如：--verbose），解析器只会跳过它们本身
    std::vector<std::string> global_flag_options;
};

class CommandParser {
public:
    // [修改] 构造函数增加 config 参数，默认为空
    explicit CommandParser(const std::vector<std::string>& args, const ParserConfig& config = {});

    std::string get_command() const;
    
    // 获取过滤了全局选项后的参数列表（主要用于 Command 内部获取位置参数）
    const std::vector<std::string>& get_filtered_args() const;
    
    std::string get_raw_arg(size_t index) const;

    // 获取带值的选项 (如 -o path)，基于原始参数搜索
    std::optional<std::string> get_option(const std::vector<std::string>& keys) const;

    // 检查布尔开关 (如 --no-save)，基于原始参数搜索
    bool has_flag(const std::vector<std::string>& keys) const;

private:
    std::vector<std::string> raw_args_;
    std::string command_;
    std::vector<std::string> filtered_args_;
    
    // [新增] 保存配置
    ParserConfig config_;

    void parse();
    
    // [修改] 不再是静态方法，依赖实例的 config_
    std::vector<std::string> filter_global_options(const std::vector<std::string>& original_args);
};

#endif // CLI_FRAMEWORK_CORE_COMMAND_PARSER_HPP_