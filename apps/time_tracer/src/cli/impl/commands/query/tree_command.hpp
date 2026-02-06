// cli/impl/commands/query/tree_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_TREE_COMMAND_H_
#define CLI_IMPL_COMMANDS_QUERY_TREE_COMMAND_H_

#include <filesystem>
#include <string>
#include <vector>

#include "cli/framework/interfaces/i_command.hpp"

class IProjectRepository;

class TreeCommand : public ICommand {
 public:
  explicit TreeCommand(std::shared_ptr<IProjectRepository> repository);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;

  auto Execute(const CommandParser& parser) -> void override;

 private:
  std::shared_ptr<IProjectRepository> repository_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_TREE_COMMAND_H_
