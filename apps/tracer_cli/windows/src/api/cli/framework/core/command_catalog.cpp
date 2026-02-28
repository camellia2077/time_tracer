// api/cli/framework/core/command_catalog.cpp
#include "api/cli/framework/core/command_catalog.hpp"

#include <array>

namespace tracer_core::cli::framework::core {

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
        .name = "chart",
        .category = "Chart",
        .summary = "Generate report-chart HTML (line/bar/pie/heatmap) from "
                   "database data.",
    },
    CommandSpec{
        .name = "crypto",
        .category = "Crypto",
        .summary = "Encrypt/decrypt/inspect raw TXT transfer files (.txt <-> "
                   ".tracer); encrypt validates structure+logic first, "
                   "passphrase is mandatory interactive, compression fixed to "
                   "zstd level 1, security level supports "
                   "interactive/moderate/high.",
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
    CommandSpec{
        .name = "doctor",
        .category = "General",
        .summary =
            "Run runtime diagnostics (core dll/config/plugins/db paths) and "
            "print structured health checks.",
    },
    CommandSpec{
        .name = "blink",
        .category = "General",
        .summary = "Legacy alias of ingest (compatibility command).",
    },
    CommandSpec{
        .name = "tracer",
        .category = "General",
        .summary = "Print the tracer easter egg line.",
    },
    CommandSpec{
        .name = "motto",
        .category = "General",
        .summary = "Print the project motto easter egg.",
    },
    CommandSpec{
        .name = "zen",
        .category = "General",
        .summary = "Alias of motto easter egg output.",
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

auto GetCommandSpecs() -> std::span<const CommandSpec> { return kCommandSpecs; }

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

} // namespace tracer_core::cli::framework::core
