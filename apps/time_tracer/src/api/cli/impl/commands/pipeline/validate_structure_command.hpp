// api/cli/impl/commands/pipeline/validate_structure_command.hpp
#ifndef CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_STRUCTURE_COMMAND_H_
#define CLI_IMPL_COMMANDS_PIPELINE_VALIDATE_STRUCTURE_COMMAND_H_

#include <filesystem>

#include "api/cli/framework/interfaces/i_command.hpp"
#include "infrastructure/config/models/app_config.hpp"

class ValidateStructureCommand : public ICommand {
 public:
  explicit ValidateStructureCommand(const AppConfig& config,
                                    std::filesystem::path output_root);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Pipeline";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  const AppConfig& app_config_;
  std::filesystem::path output_root_;
};

#endif