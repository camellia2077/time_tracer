// api/cli/impl/commands/query/tree_command.cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/tree_formatter.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "shared/types/exceptions.hpp"

class TreeCommand : public ICommand {
 public:
  explicit TreeCommand(ITimeTracerCoreApi& core_api);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;

  auto Execute(const CommandParser& parser) -> void override;

 private:
  ITimeTracerCoreApi& core_api_;
};

namespace {

auto BuildCoreErrorMessage(std::string_view fallback,
                           const std::string& error_message) -> std::string {
  if (!error_message.empty()) {
    return error_message;
  }
  return std::string(fallback);
}

void EnsureTreeResponseSuccess(
    const time_tracer::core::dto::TreeQueryResponse& response,
    std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw time_tracer::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

}  // namespace

// Register the command
static CommandRegistrar<AppContext> registrar(
    "tree", [](AppContext& ctx) -> std::unique_ptr<TreeCommand> {
      if (!ctx.core_api) {
        throw std::runtime_error("Core API not initialized");
      }
      return std::make_unique<TreeCommand>(*ctx.core_api);
    });

TreeCommand::TreeCommand(ITimeTracerCoreApi& core_api) : core_api_(core_api) {}

auto TreeCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"project_path",
           ArgType::kOption,
           {"project_path"},
           "Filter tree by project path (e.g., study, study_math)",
           false,
           ""},
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
  return "Display project structure as a tree.\n"
         "Usage: time_tracer tree [PROJECT_PATH] [options]\n\n"
         "Examples:\n"
         "  time_tracer tree                  # Show full project tree\n"
         "  time_tracer tree -r (--roots)     # List all root projects\n"
         "  time_tracer tree study            # Show subtree under 'study'\n"
         "  time_tracer tree -l 2 (--level)   # Show tree with max depth 2";
}

auto TreeCommand::Execute(const CommandParser& parser) -> void {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  if (args.Has("roots")) {
    const auto kResponse = core_api_.RunTreeQuery(
        {.list_roots = true, .root_pattern = "", .max_depth = -1});
    EnsureTreeResponseSuccess(kResponse, "Tree roots query failed.");
    TreeFormatter::PrintRoots(kResponse.roots);
    return;
  }

  int level = -1;  // unlimited
  if (args.Has("level")) {
    level = args.GetAsInt("level");
  }

  std::string root;
  // Use new API with definitions to properly skip option values
  auto positionals = parser.GetPositionalArgs(GetDefinitions());

  for (const auto& arg : positionals) {
    if (arg.find("time_tracker") != std::string::npos) {
      continue;
    }
    if (arg == "tree" || arg == "query") {
      continue;
    }

    root = arg;
    break;
  }

  const auto kResponse = core_api_.RunTreeQuery(
      {.list_roots = false, .root_pattern = root, .max_depth = level});
  EnsureTreeResponseSuccess(kResponse, "Tree query failed.");
  if (!kResponse.found) {
    std::cerr << "No project found matching path: " << root << "\n";
    return;
  }

  TreeFormatter::PrintTree(kResponse.nodes);
}
