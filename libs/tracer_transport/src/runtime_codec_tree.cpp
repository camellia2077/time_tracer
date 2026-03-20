#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

import tracer.transport.fields;

namespace tracer::transport {

namespace {

using nlohmann::json;
using tracer::transport::modfields::RequireStringField;
using tracer::transport::modfields::TryReadBoolField;
using tracer::transport::modfields::TryReadIntField;
using tracer::transport::modfields::TryReadStringField;

auto ParseRequestObject(std::string_view request_json) -> json {
  if (request_json.empty()) {
    throw std::invalid_argument("request_json must not be empty.");
  }
  json payload = json::parse(std::string(request_json));
  if (!payload.is_object()) {
    throw std::invalid_argument("request_json must be a JSON object.");
  }
  return payload;
}

auto ParseResponseObject(std::string_view response_json) -> json {
  if (response_json.empty()) {
    throw std::invalid_argument("response_json must not be empty.");
  }
  json payload = json::parse(std::string(response_json));
  if (!payload.is_object()) {
    throw std::invalid_argument("response_json must be a JSON object.");
  }
  return payload;
}

auto SerializeTreeNode(const ProjectTreeNodePayload& node) -> json {
  json children = json::array();
  for (const auto& child : node.children) {
    children.push_back(SerializeTreeNode(child));
  }
  json out = json{
      {"name", node.name},
      {"children", std::move(children)},
  };
  if (node.path.has_value()) {
    out["path"] = *node.path;
  }
  if (node.duration_seconds.has_value()) {
    out["duration_seconds"] = *node.duration_seconds;
  }
  return out;
}

auto ParseTreeNode(const json& node_json) -> ProjectTreeNodePayload {
  if (!node_json.is_object()) {
    throw std::invalid_argument("field `nodes` must be an object array.");
  }

  const auto kName = RequireStringField(node_json, "name");
  if (kName.HasError()) {
    throw std::invalid_argument(kName.error.message);
  }

  ProjectTreeNodePayload out{};
  out.name = kName.value.value_or("");

  const auto kPath = TryReadStringField(node_json, "path");
  if (kPath.HasError()) {
    throw std::invalid_argument(kPath.error.message);
  }
  out.path = kPath.value;

  const auto kDurationSecondsIt = node_json.find("duration_seconds");
  if (kDurationSecondsIt != node_json.end() && !kDurationSecondsIt->is_null()) {
    if (!kDurationSecondsIt->is_number_integer()) {
      throw std::invalid_argument("field `duration_seconds` must be an integer.");
    }
    out.duration_seconds = kDurationSecondsIt->get<long long>();
  }

  const auto kChildrenIt = node_json.find("children");
  if (kChildrenIt == node_json.end() || kChildrenIt->is_null()) {
    return out;
  }
  if (!kChildrenIt->is_array()) {
    throw std::invalid_argument("field `children` must be an object array.");
  }

  out.children.reserve(kChildrenIt->size());
  for (const auto& child : *kChildrenIt) {
    out.children.push_back(ParseTreeNode(child));
  }
  return out;
}

struct TreeResponseEnvelopeFields {
  bool ok = false;
  bool found = true;
  std::string error_message;
  std::string error_code;
  std::string error_category;
};

auto ParseTreeResponseEnvelopeFields(const json& payload)
    -> TreeResponseEnvelopeFields {
  const auto kOk = TryReadBoolField(payload, "ok");
  const auto kFound = TryReadBoolField(payload, "found");
  const auto kErrorMessage = TryReadStringField(payload, "error_message");
  const auto kErrorCode = TryReadStringField(payload, "error_code");
  const auto kErrorCategory = TryReadStringField(payload, "error_category");
  if (kOk.HasError()) {
    throw std::invalid_argument(kOk.error.message);
  }
  if (!kOk.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (kFound.HasError()) {
    throw std::invalid_argument(kFound.error.message);
  }
  if (kErrorMessage.HasError()) {
    throw std::invalid_argument(kErrorMessage.error.message);
  }
  if (kErrorCode.HasError()) {
    throw std::invalid_argument(kErrorCode.error.message);
  }
  if (kErrorCategory.HasError()) {
    throw std::invalid_argument(kErrorCategory.error.message);
  }
  return TreeResponseEnvelopeFields{
      .ok = *kOk.value,
      .found = kFound.value.value_or(true),
      .error_message = kErrorMessage.value.value_or(""),
      .error_code = kErrorCode.value.value_or(""),
      .error_category = kErrorCategory.value.value_or(""),
  };
}

auto ParseOptionalRoots(const json& payload) -> std::vector<std::string> {
  if (const auto kRootsIt = payload.find("roots");
      kRootsIt != payload.end() && !kRootsIt->is_null()) {
    if (!kRootsIt->is_array()) {
      throw std::invalid_argument("field `roots` must be a string array.");
    }
    std::vector<std::string> roots;
    roots.reserve(kRootsIt->size());
    for (const auto& root : *kRootsIt) {
      if (!root.is_string()) {
        throw std::invalid_argument("field `roots` must be a string array.");
      }
      roots.push_back(root.get<std::string>());
    }
    return roots;
  }
  return {};
}

auto ParseOptionalResponseNodes(const json& payload)
    -> std::vector<ProjectTreeNodePayload> {
  if (const auto kNodesIt = payload.find("nodes");
      kNodesIt != payload.end() && !kNodesIt->is_null()) {
    if (!kNodesIt->is_array()) {
      throw std::invalid_argument("field `nodes` must be an object array.");
    }
    std::vector<ProjectTreeNodePayload> nodes;
    nodes.reserve(kNodesIt->size());
    for (const auto& node : *kNodesIt) {
      nodes.push_back(ParseTreeNode(node));
    }
    return nodes;
  }
  return {};
}

auto ParseOptionalHints(const json& payload) -> std::vector<std::string> {
  if (const auto kHintsIt = payload.find("hints");
      kHintsIt != payload.end() && !kHintsIt->is_null()) {
    if (!kHintsIt->is_array()) {
      throw std::invalid_argument("field `hints` must be a string array.");
    }
    std::vector<std::string> hints;
    hints.reserve(kHintsIt->size());
    for (const auto& hint : *kHintsIt) {
      if (!hint.is_string()) {
        throw std::invalid_argument("field `hints` must be a string array.");
      }
      hints.push_back(hint.get<std::string>());
    }
    return hints;
  }
  return {};
}

}  // namespace

auto DecodeTreeRequest(std::string_view request_json) -> TreeRequestPayload {
  const json kPayload = ParseRequestObject(request_json);

  const auto kListRoots = TryReadBoolField(kPayload, "list_roots");
  if (kListRoots.HasError()) {
    throw std::invalid_argument(kListRoots.error.message);
  }
  const auto kRootPattern = TryReadStringField(kPayload, "root_pattern");
  if (kRootPattern.HasError()) {
    throw std::invalid_argument(kRootPattern.error.message);
  }
  const auto kMaxDepth = TryReadIntField(kPayload, "max_depth");
  if (kMaxDepth.HasError()) {
    throw std::invalid_argument(kMaxDepth.error.message);
  }
  const auto kPeriod = TryReadStringField(kPayload, "period");
  if (kPeriod.HasError()) {
    throw std::invalid_argument(kPeriod.error.message);
  }
  const auto kPeriodArgument = TryReadStringField(kPayload, "period_argument");
  if (kPeriodArgument.HasError()) {
    throw std::invalid_argument(kPeriodArgument.error.message);
  }
  const auto kRoot = TryReadStringField(kPayload, "root");
  if (kRoot.HasError()) {
    throw std::invalid_argument(kRoot.error.message);
  }

  TreeRequestPayload out{};
  out.list_roots = kListRoots.value;
  out.root_pattern = kRootPattern.value;
  out.max_depth = kMaxDepth.value;
  out.period = kPeriod.value;
  out.period_argument = kPeriodArgument.value;
  out.root = kRoot.value;
  return out;
}

auto EncodeTreeRequest(const TreeRequestPayload& request) -> std::string {
  json payload = json::object();
  if (request.list_roots.has_value()) {
    payload["list_roots"] = *request.list_roots;
  }
  if (request.root_pattern.has_value()) {
    payload["root_pattern"] = *request.root_pattern;
  }
  if (request.max_depth.has_value()) {
    payload["max_depth"] = *request.max_depth;
  }
  if (request.period.has_value()) {
    payload["period"] = *request.period;
  }
  if (request.period_argument.has_value()) {
    payload["period_argument"] = *request.period_argument;
  }
  if (request.root.has_value()) {
    payload["root"] = *request.root;
  }
  return payload.dump();
}

auto DecodeTreeResponse(std::string_view response_json) -> TreeResponsePayload {
  const json kPayload = ParseResponseObject(response_json);
  const TreeResponseEnvelopeFields kEnvelope =
      ParseTreeResponseEnvelopeFields(kPayload);

  TreeResponsePayload out{};
  out.ok = kEnvelope.ok;
  out.found = kEnvelope.found;
  out.error_message = kEnvelope.error_message;
  out.error_contract.error_code = kEnvelope.error_code;
  out.error_contract.error_category = kEnvelope.error_category;
  out.roots = ParseOptionalRoots(kPayload);
  out.nodes = ParseOptionalResponseNodes(kPayload);
  out.error_contract.hints = ParseOptionalHints(kPayload);

  return out;
}

auto EncodeTreeResponse(const TreeResponsePayload& response) -> std::string {
  json roots = json::array();
  for (const auto& root : response.roots) {
    roots.push_back(root);
  }

  json nodes = json::array();
  for (const auto& node : response.nodes) {
    nodes.push_back(SerializeTreeNode(node));
  }

  return json{
      {"ok", response.ok},
      {"found", response.found},
      {"roots", std::move(roots)},
      {"nodes", std::move(nodes)},
      {"error_message", response.error_message},
      {"error_code", response.error_contract.error_code},
      {"error_category", response.error_contract.error_category},
      {"hints", response.error_contract.hints},
  }
      .dump();
}

}  // namespace tracer::transport
