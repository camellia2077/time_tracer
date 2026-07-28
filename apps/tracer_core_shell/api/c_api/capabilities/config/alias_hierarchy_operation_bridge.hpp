#ifndef API_C_API_CAPABILITIES_CONFIG_ALIAS_HIERARCHY_OPERATION_BRIDGE_HPP_
#define API_C_API_CAPABILITIES_CONFIG_ALIAS_HIERARCHY_OPERATION_BRIDGE_HPP_

#include <nlohmann/json_fwd.hpp>

namespace tracer_core::shell::config_bridge {

// Maps the runtime JSON request to the core-owned in-memory hierarchy
// operation. The returned object contains only operation payload fields; the
// C ABI entrypoint owns the standard response envelope.
[[nodiscard]] auto ApplyAliasHierarchyOperationJson(
    const nlohmann::json& payload) -> nlohmann::json;

[[nodiscard]] auto RewriteAliasHierarchyDocumentJson(
    const nlohmann::json& payload) -> nlohmann::json;

// Produces the validated core hierarchy snapshot for presentation without
// applying an edit.
[[nodiscard]] auto DescribeAliasHierarchyJson(const nlohmann::json& payload)
    -> nlohmann::json;

[[nodiscard]] auto ValidateAliasHierarchyDocumentsJson(
    const nlohmann::json& payload) -> nlohmann::json;

[[nodiscard]] auto CreateAliasHierarchyDocumentJson(const nlohmann::json& payload)
    -> nlohmann::json;

}  // namespace tracer_core::shell::config_bridge

#endif  // API_C_API_CAPABILITIES_CONFIG_ALIAS_HIERARCHY_OPERATION_BRIDGE_HPP_
