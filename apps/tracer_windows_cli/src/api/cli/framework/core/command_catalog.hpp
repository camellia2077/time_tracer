#pragma once

#include <string>
#include <string_view>

namespace time_tracer::cli::framework::core {

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

} // namespace time_tracer::cli::framework::core
