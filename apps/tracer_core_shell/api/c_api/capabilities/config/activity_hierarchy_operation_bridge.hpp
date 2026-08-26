#ifndef API_C_API_CAPABILITIES_CONFIG_ACTIVITY_HIERARCHY_OPERATION_BRIDGE_HPP_
#define API_C_API_CAPABILITIES_CONFIG_ACTIVITY_HIERARCHY_OPERATION_BRIDGE_HPP_

#include <nlohmann/json_fwd.hpp>

namespace tracer_core::shell::config_bridge {

// Maps the runtime JSON request to the core-owned in-memory hierarchy
// operation. The returned object contains only operation payload fields; the
// C ABI entrypoint owns the standard response envelope.
[[nodiscard]] auto ApplyActivityHierarchyOperationJson(
    const nlohmann::json& payload) -> nlohmann::json;

[[nodiscard]] auto MoveActivityHierarchyLeafBetweenDocumentsJson(
    const nlohmann::json& payload) -> nlohmann::json;

[[nodiscard]] auto MoveActivityHierarchyNodeBetweenDocumentsJson(
    const nlohmann::json& payload) -> nlohmann::json;

[[nodiscard]] auto RewriteActivityHierarchyDocumentJson(
    const nlohmann::json& payload) -> nlohmann::json;

// Produces the validated core hierarchy snapshot for presentation without
// applying an edit.
[[nodiscard]] auto DescribeActivityHierarchyJson(const nlohmann::json& payload)
    -> nlohmann::json;

[[nodiscard]] auto SearchActivityHierarchyJson(const nlohmann::json& payload)
    -> nlohmann::json;

[[nodiscard]] auto ValidateActivityHierarchyDocumentsJson(
    const nlohmann::json& payload) -> nlohmann::json;

}  // namespace tracer_core::shell::config_bridge

#endif  // API_C_API_CAPABILITIES_CONFIG_ACTIVITY_HIERARCHY_OPERATION_BRIDGE_HPP_
