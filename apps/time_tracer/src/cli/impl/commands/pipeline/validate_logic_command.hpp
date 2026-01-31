// cli/impl/commands/pipeline/validate_logic_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_LOGIC_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_LOGIC_COMMAND_H_

#include <filesystem>

#include "cli/framework/interfaces/i_command.hpp"
#include "common/config/app_config.hpp"

// [重命名] ValidateOutputCommand -> ValidateLogicCommand
class ValidateLogicCommand : public ICommand {
 public:
  explicit ValidateLogicCommand(const AppConfig& config,
                                std::filesystem::path output_root);

  std::vector<ArgDef> get_definitions() const override;
  std::string get_help() const override;
  void execute(const CommandParser& parser) override;

 private:
  const AppConfig& app_config_;
  std::filesystem::path output_root_;
};

#endif