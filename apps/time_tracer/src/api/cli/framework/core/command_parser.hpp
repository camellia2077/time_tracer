// api/cli/framework/core/command_parser.hpp
#ifndef API_CLI_FRAMEWORK_CORE_COMMAND_PARSER_H_
#define API_CLI_FRAMEWORK_CORE_COMMAND_PARSER_H_

#include <optional>
#include <span>
#include <string>
#include <vector>

#include "api/cli/framework/core/arg_definitions.hpp"

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
  explicit CommandParser(std::vector<std::string> args,
                         ParserConfig config = {});

  [[nodiscard]] auto GetCommand() const -> std::string;
  [[nodiscard]] auto GetFilteredArgs() const -> const std::vector<std::string>&;

  // 简单版本：只过滤以 '-' 开头的参数（不推荐，可能包含选项值）
  [[nodiscard]] auto GetPositionalArgs() const -> std::vector<std::string>;

  // 智能版本：根据参数定义跳过 Option 类型的值
  [[nodiscard]] auto GetPositionalArgs(
      std::span<const ArgDef> definitions) const -> std::vector<std::string>;

  [[nodiscard]] auto GetRawArg(size_t index) const -> std::string;

  [[nodiscard]] auto GetOption(const std::vector<std::string>& keys) const
      -> std::optional<std::string>;

  [[nodiscard]] auto HasFlag(const std::vector<std::string>& keys) const
      -> bool;

 private:
  std::vector<std::string> raw_args_;
  std::string command_;
  std::vector<std::string> filtered_args_;

  ParserConfig config_;

  void Parse();

  auto FilterGlobalOptions(const std::vector<std::string>& original_args)
      -> std::vector<std::string>;
};

#endif  // API_CLI_FRAMEWORK_CORE_COMMAND_PARSER_H_