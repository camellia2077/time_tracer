// cli/framework/core/arg_definitions.hpp
#ifndef CLI_FRAMEWORK_CORE_ARG_DEFINITIONS_H_
#define CLI_FRAMEWORK_CORE_ARG_DEFINITIONS_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class ArgType {
  Positional,  // 如: ingest <path> 中的 <path>
  Option,      // 如: --format md
  Flag         // 如: --verbose
};

struct ArgDef {
  std::string name;  // 内部使用的键名，如 "input_path"
  ArgType type;
  std::vector<std::string>
      keys;          // 触发键，如 {"-f", "--format"} (仅 Option/Flag 有效)
  std::string help;  // 帮助文本
  bool required = false;
  std::string default_value = "";

  // 用于位置参数：指定它在 filtered_args
  // 中的索引（偏移量，跳过程序名和命令名后） 例如：convert <path> -> index 0
  size_t position_index = 0;
};

// 解析后的结果容器
class ParsedArgs {
 public:
  std::string get(const std::string& name) const;  // 获取值
  bool has(const std::string& name) const;         // 是否存在
  int get_as_int(const std::string& name) const;   // 类型转换助手
 private:
  std::map<std::string, std::string> values_;
  friend class CommandValidator;
};

#endif