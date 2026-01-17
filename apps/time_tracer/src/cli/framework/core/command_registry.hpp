// cli/framework/core/command_registry.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_
#define CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include "cli/framework/interfaces/i_command.hpp"

// [解耦] 不再包含 AppContext，Framework 不再认识具体的 Context

template <typename ContextT>
class CommandRegistry {
public:
    // 定义工厂函数类型，依赖于模板参数 ContextT
    using ICommandCreator = std::function<std::unique_ptr<ICommand>(ContextT&)>;

    // 单例访问
    static CommandRegistry& Instance() {
        static CommandRegistry inst;
        return inst;
    }

    // 注册命令
    void RegisterCommand(const std::string& name, ICommandCreator creator) {
        creators_[name] = creator;
    }

    // 创建命令
    std::unique_ptr<ICommand> CreateCommand(const std::string& name, ContextT& ctx) {
        auto it = creators_.find(name);
        if (it != creators_.end()) {
            return it->second(ctx);
        }
        return nullptr;
    }

    // 获取所有已注册的命令名称
    std::vector<std::string> GetRegisteredCommands() const {
        std::vector<std::string> names;
        for (const auto& pair : creators_) {
            names.push_back(pair.first);
        }
        return names;
    }

private:
    CommandRegistry() = default;
    std::map<std::string, ICommandCreator> creators_;
};

// 辅助类：模板化的注册器
template <typename ContextT>
struct CommandRegistrar {
    CommandRegistrar(const std::string& name, typename CommandRegistry<ContextT>::ICommandCreator creator) {
        CommandRegistry<ContextT>::Instance().RegisterCommand(name, creator);
    }
};

#endif // CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_