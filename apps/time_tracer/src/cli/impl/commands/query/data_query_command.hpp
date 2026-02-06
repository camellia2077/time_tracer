// cli/impl/commands/query/data_query_command.hpp
#ifndef CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_COMMAND_H_
#define CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_COMMAND_H_

#include <filesystem>
#include <string>
#include <vector>

#include "cli/framework/interfaces/i_command.hpp"

struct AppContext;

class DataQueryCommand : public ICommand {
 public:
  explicit DataQueryCommand(std::filesystem::path db_path);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;

  auto Execute(const CommandParser& parser) -> void override;

 private:
  std::filesystem::path db_path_;
};

#endif  // CLI_IMPL_COMMANDS_QUERY_DATA_QUERY_COMMAND_H_
