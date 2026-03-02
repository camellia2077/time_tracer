// api/cli/framework/core/command_catalog.hpp
#pragma once

#include <span>
#include <string>
#include <string_view>

namespace tracer_core::cli::framework::core {

struct CommandSpec {
  std::string_view name;
  std::string_view category;
  std::string_view summary;
};

struct DeprecatedOptionSpec {
  std::string_view command_name;
  std::string_view option_key;
  std::string_view message;
};

[[nodiscard]] auto FindCommandSpec(std::string_view command_name)
    -> const CommandSpec *;

[[nodiscard]] auto GetCommandSpecs() -> std::span<const CommandSpec>;

[[nodiscard]] auto ResolveCommandCategory(std::string_view command_name,
                                          std::string_view fallback)
    -> std::string;

[[nodiscard]] auto ResolveCommandSummary(std::string_view command_name,
                                         std::string_view fallback)
    -> std::string;

[[nodiscard]] auto FindDeprecatedOptionSpec(std::string_view command_name,
                                            std::string_view option_key)
    -> const DeprecatedOptionSpec *;

[[nodiscard]] auto ResolveDeprecatedOptionHelp(std::string_view command_name,
                                               std::string_view option_key,
                                               std::string_view fallback)
    -> std::string;

} // namespace tracer_core::cli::framework::core
