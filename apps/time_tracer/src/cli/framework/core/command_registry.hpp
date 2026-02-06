// cli/framework/core/command_registry.hpp
#ifndef CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_H_
#define CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cli/framework/interfaces/i_command.hpp"

template <typename ContextT>
class CommandRegistry {
 public:
  using ICommandCreator = std::function<std::unique_ptr<ICommand>(ContextT&)>;

  static auto Instance() -> CommandRegistry& {
    static CommandRegistry inst;
    return inst;
  }

  void RegisterCommand(const std::string& name, ICommandCreator creator) {
    creators_[name] = creator;
  }

  auto CreateCommand(const std::string& name, ContextT& ctx)
      -> std::unique_ptr<ICommand> {
    auto iter = creators_.find(name);
    if (iter != creators_.end()) {
      return iter->second(ctx);
    }
    return nullptr;
  }

  // [新增] 批量创建所有已注册的命令实例，用于生成帮助文档
  // [新增] 批量创建所有已注册的命令实例，用于生成帮助文档
  auto CreateAllCommands(ContextT& ctx)
      -> std::vector<std::pair<std::string, std::unique_ptr<ICommand>>> {
    std::vector<std::pair<std::string, std::unique_ptr<ICommand>>> commands;
    for (const auto& pair : creators_) {
      if (pair.second) {
        commands.push_back({pair.first, pair.second(ctx)});
      }
    }
    return commands;
  }

  [[nodiscard]] auto GetRegisteredCommands() const -> std::vector<std::string> {
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
  CommandRegistrar(
      const std::string& name,
      typename CommandRegistry<ContextT>::ICommandCreator creator) {
    CommandRegistry<ContextT>::Instance().RegisterCommand(name, creator);
  }
};

#endif  // CLI_FRAMEWORK_CORE_COMMAND_REGISTRY_H_