#include "api/cli/framework/core/command_catalog.hpp"

#include <array>

namespace time_tracer::cli::framework::core {

namespace {

constexpr std::array kCommandSpecs = {
    CommandSpec{
        .name = "query",
        .category = "Query",
        .summary =
            "Query statistics (day, month, week, year, recent, range) from "
            "the database. For query data tree/days-stats/report-chart, "
            "prefer --root over legacy --project.",
    },
    CommandSpec{
        .name = "export",
        .category = "Export",
        .summary = "Export reports (day, month, week, year, recent, etc.) to "
                   "specified formats (md, tex, typ).",
    },
    CommandSpec{
        .name = "convert",
        .category = "Pipeline",
        .summary =
            "Convert source files (e.g., .txt) to processed JSON format.",
    },
    CommandSpec{
        .name = "import",
        .category = "Pipeline",
        .summary = "Import processed JSON data into the database.",
    },
    CommandSpec{
        .name = "ingest",
        .category = "Pipeline",
        .summary =
            "Run the full data ingestion pipeline: validate source -> convert "
            "-> validate logic -> import.",
    },
    CommandSpec{
        .name = "validate-logic",
        .category = "Pipeline",
        .summary =
            "Validate business logic (e.g., date continuity, sleep cycles). "
            "Requires conversion but won't save output.",
    },
    CommandSpec{
        .name = "validate-structure",
        .category = "Pipeline",
        .summary = "Validate the syntax and structure of source TXT files "
                   "(Read-only).",
    },
    CommandSpec{
        .name = "tree",
        .category = "General",
        .summary = "Display project structure as a tree.",
    },
};

constexpr std::array kDeprecatedOptionSpecs = {
    DeprecatedOptionSpec{
        .command_name = "query",
        .option_key = "--project",
        .message =
            "Legacy contains filter by project path (prefer --root for root "
            "scope)",
    },
};

} // namespace

auto FindCommandSpec(std::string_view command_name) -> const CommandSpec * {
  for (const auto &spec : kCommandSpecs) {
    if (spec.name == command_name) {
      return &spec;
    }
  }
  return nullptr;
}

auto ResolveCommandCategory(std::string_view command_name,
                            std::string_view fallback) -> std::string {
  if (const CommandSpec *spec = FindCommandSpec(command_name);
      spec != nullptr && !spec->category.empty()) {
    return std::string(spec->category);
  }
  return std::string(fallback);
}

auto ResolveCommandSummary(std::string_view command_name,
                           std::string_view fallback) -> std::string {
  if (const CommandSpec *spec = FindCommandSpec(command_name);
      spec != nullptr && !spec->summary.empty()) {
    return std::string(spec->summary);
  }
  return std::string(fallback);
}

auto FindDeprecatedOptionSpec(std::string_view command_name,
                              std::string_view option_key)
    -> const DeprecatedOptionSpec * {
  for (const auto &spec : kDeprecatedOptionSpecs) {
    if (spec.command_name == command_name && spec.option_key == option_key) {
      return &spec;
    }
  }
  return nullptr;
}

auto ResolveDeprecatedOptionHelp(std::string_view command_name,
                                 std::string_view option_key,
                                 std::string_view fallback) -> std::string {
  if (const DeprecatedOptionSpec *spec =
          FindDeprecatedOptionSpec(command_name, option_key);
      spec != nullptr && !spec->message.empty()) {
    return std::string(spec->message);
  }
  return std::string(fallback);
}

} // namespace time_tracer::cli::framework::core
