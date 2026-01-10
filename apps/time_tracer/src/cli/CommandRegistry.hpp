// cli/CommandRegistry.hpp
#ifndef COMMAND_REGISTRY_HPP
#define COMMAND_REGISTRY_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "cli/commands/interface/ICommand.hpp"
#include "cli/CliContext.hpp"

// 定义工厂函数类型
using CommandCreator = std::function<std::unique_ptr<ICommand>(CliContext&)>;

class CommandRegistry {
public:
    // 单例访问
    static CommandRegistry& instance();

    // 注册命令
    void register_command(const std::string& name, CommandCreator creator);

    // 创建命令
    std::unique_ptr<ICommand> create_command(const std::string& name, CliContext& ctx);

    // 获取所有已注册的命令名称（可选，用于帮助文档）
    std::vector<std::string> get_registered_commands() const;

private:
    CommandRegistry() = default;
    std::map<std::string, CommandCreator> creators_;
};

// 辅助类，用于在静态初始化期间注册命令
struct CommandRegistrar {
    CommandRegistrar(const std::string& name, CommandCreator creator);
};

#endif // COMMAND_REGISTRY_HPP