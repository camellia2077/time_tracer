// api/cli/impl/utils/help_formatter.hpp
#ifndef CLI_IMPL_UTILS_HELP_FORMATTER_H_
#define CLI_IMPL_UTILS_HELP_FORMATTER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

// 前向声明
class ICommand;

// [修改] 接收命令列表以动态生成帮助
void PrintFullUsage(

    const char* app_name,
    const std::vector<std::pair<std::string, std::unique_ptr<ICommand>>>&
        commands);

// [新增] 打印单个命令的帮助
void PrintCommandUsage(const std::string& command_name,
                       const ICommand& command);

#endif  // CLI_IMPL_UTILS_HELP_FORMATTER_H_