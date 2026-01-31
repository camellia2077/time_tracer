// cli/impl/utils/help_formatter.hpp
#ifndef CLI_IMPL_UTILS_HELP_FORMATTER_H_
#define CLI_IMPL_UTILS_HELP_FORMATTER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

// 前向声明
class ICommand;

// [修改] 接收命令列表以动态生成帮助
void print_full_usage(
    const char* app_name,
    const std::vector<std::pair<std::string, std::unique_ptr<ICommand>>>&
        commands);

#endif  // CLI_IMPL_UTILS_HELP_FORMATTER_H_