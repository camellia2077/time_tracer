// cli/framework/interfaces/i_command.hpp
#ifndef CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_
#define CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_

#include <string>
#include <vector>
#include "cli/framework/core/arg_definitions.hpp"

// [关键修复 1] 前向声明 CommandParser，解决 "does not name a type" 错误
class CommandParser;

class ICommand {
public:
    virtual ~ICommand() = default;
    
    // 子类必须实现此方法，描述它需要的参数
    virtual std::vector<ArgDef> get_definitions() const = 0;

    // [关键修复 1 的配套] 这里使用 CommandParser& 引用，配合前向声明即可编译
    virtual void execute(const CommandParser& parser) = 0;
    
    // 默认的 get_help
    virtual std::string get_help() const {
        return "Auto generated help..."; 
    }
};

#endif // CLI_FRAMEWORK_INTERFACES_I_COMMAND_HPP_