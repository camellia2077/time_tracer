// cli/impl/commands/query/tree_command.cpp
#include "cli/impl/commands/query/tree_command.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "application/reporting/tree/project_tree_viewer.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "cli/impl/app/app_context.hpp"
#include "cli/impl/utils/tree_formatter.hpp"

// Register the command
static CommandRegistrar<AppContext> registrar(
    "tree", [](AppContext& ctx) -> std::unique_ptr<TreeCommand> {
      return std::make_unique<TreeCommand>(ctx.project_repository);
    });

TreeCommand::TreeCommand(std::shared_ptr<IProjectRepository> repository)
    : repository_(std::move(repository)) {}

auto TreeCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"project_path", ArgType::kOption, {"project_path"}, "Filter tree by project path (e.g., study, study_math)"},
      {"level", ArgType::kOption, {"-l", "--level"}, "Max depth level"},
      {"roots", ArgType::kFlag, {"-r", "--roots"}, "List all root projects"}
  };
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

  ProjectTreeViewer viewer(repository_);

  if (args.Has("roots")) {
    auto roots = viewer.GetRoots();
    TreeFormatter::PrintRoots(roots);
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

  auto tree_result = viewer.GetTree(root, level);
  if (!tree_result.has_value()) {
    std::cerr << "No project found matching path: " << root << "\n";
    return;
  }

  TreeFormatter::PrintTree(*tree_result);
}
