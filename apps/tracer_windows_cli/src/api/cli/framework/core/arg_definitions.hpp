// api/cli/framework/core/arg_definitions.hpp
#ifndef API_CLI_FRAMEWORK_CORE_ARG_DEFINITIONS_H_
#define API_CLI_FRAMEWORK_CORE_ARG_DEFINITIONS_H_

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class ArgType {
  kPositional, // 如: ingest <path> 中的 <path>
  kOption,     // 如: --format md
  kFlag        // 如: --verbose
};

struct ArgDef {
  std::string name; // 内部使用的键名，如 "input_path"
  ArgType type;
  std::vector<std::string>
      keys;         // 触发键，如 {"-f", "--format"} (仅 Option/Flag 有效)
  std::string help; // 帮助文本
  bool required = false;
  std::string default_value;

  // 用于位置参数：指定它在 filtered_args
  // 中的索引（偏移量，跳过程序名和命令名后） 例如：convert <path> -> index 0
  size_t position_index = 0;
};

// 解析后的结果容器
class ParsedArgs {
public:
  [[nodiscard]] auto Get(const std::string &name) const
      -> std::string;                                            // 获取值
  [[nodiscard]] auto Has(const std::string &name) const -> bool; // 是否存在
  [[nodiscard]] auto GetAsInt(const std::string &name) const
      -> int; // 类型转换助手
private:
  std::map<std::string, std::string> values_;
  friend class CommandValidator;
};

#endif