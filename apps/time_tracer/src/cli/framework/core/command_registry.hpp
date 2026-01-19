// cli/framework/core/command_registry.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_
#define CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include "cli/framework/interfaces/i_command.hpp"

template <typename ContextT>
class CommandRegistry {
public:
    using ICommandCreator = std::function<std::unique_ptr<ICommand>(ContextT&)>;

    static CommandRegistry& Instance() {
        static CommandRegistry inst;
        return inst;
    }

    void RegisterCommand(const std::string& name, ICommandCreator creator) {
        creators_[name] = creator;
    }

    std::unique_ptr<ICommand> CreateCommand(const std::string& name, ContextT& ctx) {
        auto it = creators_.find(name);
        if (it != creators_.end()) {
            return it->second(ctx);
        }
        return nullptr;
    }

    // [新增] 批量创建所有已注册的命令实例，用于生成帮助文档
    std::vector<std::pair<std::string, std::unique_ptr<ICommand>>> CreateAllCommands(ContextT& ctx) {
        std::vector<std::pair<std::string, std::unique_ptr<ICommand>>> commands;
        for (const auto& pair : creators_) {
            if (pair.second) {
                commands.push_back({pair.first, pair.second(ctx)});
            }
        }
        return commands;
    }

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

template <typename ContextT>
struct CommandRegistrar {
    CommandRegistrar(const std::string& name, typename CommandRegistry<ContextT>::ICommandCreator creator) {
        CommandRegistry<ContextT>::Instance().RegisterCommand(name, creator);
    }
};

#endif // CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_HPP_