#include "application/ports/config/alias_tree_text_renderer.hpp"

#include "infra/config/loader/alias_tree_text_renderer.hpp"

namespace tracer::core::application::config {

auto RenderAliasTreeText(const std::filesystem::path& alias_toml_path,
                         bool show_aliases) -> std::string {
  return infrastructure::config::loader::detail::RenderAliasTreeText(
      alias_toml_path, show_aliases);
}

auto RenderAliasTreeText(std::string_view toml_content, bool show_aliases)
    -> std::string {
  return infrastructure::config::loader::detail::RenderAliasTreeText(
      infrastructure::config::loader::detail::ParseAliasDocument(
          toml::parse(toml_content)),
      show_aliases);
}

}  // namespace tracer::core::application::config
