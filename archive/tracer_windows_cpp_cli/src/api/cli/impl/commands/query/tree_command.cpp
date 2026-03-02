// api/cli/impl/commands/query/tree_command.cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/tree_formatter.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"

class TreeCommand : public ICommand {
public:
  explicit TreeCommand(ITracerCoreApi &core_api);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "tree", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
};

namespace {

auto BuildCoreErrorMessage(std::string_view fallback,
                           const std::string &error_message) -> std::string {
  if (!error_message.empty()) {
    return error_message;
  }
  return std::string(fallback);
}

void EnsureTreeResponseSuccess(
    const tracer_core::core::dto::TreeQueryResponse &response,
    std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw tracer_core::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterTreeCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "tree", [](AppContext &ctx) -> std::unique_ptr<TreeCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        return std::make_unique<TreeCommand>(*ctx.core_api);
      });
}

} // namespace tracer_core::cli::impl::commands

TreeCommand::TreeCommand(ITracerCoreApi &core_api) : core_api_(core_api) {}

auto TreeCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"root",
           ArgType::kPositional,
           {},
           "Root project path filter (e.g., study, study_math)",
           false,
           "",
           0},
          {"level",
           ArgType::kOption,
           {"-l", "--level"},
           "Max depth level",
           false,
           ""},
          {"roots",
           ArgType::kFlag,
           {"-r", "--roots"},
           "List all root projects",
           false,
           ""}};
}

auto TreeCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "tree", ICommand::GetHelp());
}

auto TreeCommand::Execute(const CommandParser &parser) -> void {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());
  const auto positionals = parser.GetPositionalArgs(GetDefinitions());

  if (positionals.size() > 1U) {
    throw std::runtime_error(
        "tree accepts at most one <root> positional argument.");
  }
  const std::string root = positionals.empty() ? std::string{} : positionals[0];

  if (args.Has("roots")) {
    if (!root.empty()) {
      throw std::runtime_error(
          "tree --roots does not accept <root> positional argument.");
    }
    const auto kResponse = core_api_.RunTreeQuery(
        {.list_roots = true, .root_pattern = "", .max_depth = -1});
    EnsureTreeResponseSuccess(kResponse, "Tree roots query failed.");
    TreeFormatter::PrintRoots(kResponse.roots);
    return;
  }

  int level = -1; // unlimited
  if (args.Has("level")) {
    level = args.GetAsInt("level");
  }

  const auto kResponse = core_api_.RunTreeQuery(
      {.list_roots = false, .root_pattern = root, .max_depth = level});
  EnsureTreeResponseSuccess(kResponse, "Tree query failed.");
  if (!kResponse.found) {
    namespace colors = tracer_core::common::colors;
    if (root.empty()) {
      std::cerr << colors::kYellow << "No project entries were found."
                << colors::kReset << "\n";
    } else {
      std::cerr << colors::kYellow << "No project found matching path: " << root
                << "." << colors::kReset << "\n";
    }
    std::cerr << colors::kGray
              << "Use 'tree -r' to list available root projects."
              << colors::kReset << "\n";
    return;
  }

  TreeFormatter::PrintTree(kResponse.nodes);
}
