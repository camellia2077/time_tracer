#include "tracer/transport/runtime_codec.hpp"

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"
#include "tracer/transport/fields.hpp"

namespace tracer::transport {

namespace {

using nlohmann::json;

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
  return json{
      {"name", node.name},
      {"children", std::move(children)},
  };
}

auto ParseTreeNode(const json& node_json) -> ProjectTreeNodePayload {
  if (!node_json.is_object()) {
    throw std::invalid_argument("field `nodes` must be an object array.");
  }

  const auto name = RequireStringField(node_json, "name");
  if (name.HasError()) {
    throw std::invalid_argument(name.error.message);
  }

  ProjectTreeNodePayload out{};
  out.name = name.value.value_or("");

  const auto children_it = node_json.find("children");
  if (children_it == node_json.end() || children_it->is_null()) {
    return out;
  }
  if (!children_it->is_array()) {
    throw std::invalid_argument("field `children` must be an object array.");
  }

  out.children.reserve(children_it->size());
  for (const auto& child : *children_it) {
    out.children.push_back(ParseTreeNode(child));
  }
  return out;
}

}  // namespace

auto DecodeTreeRequest(std::string_view request_json) -> TreeRequestPayload {
  const json payload = ParseRequestObject(request_json);

  const auto list_roots = TryReadBoolField(payload, "list_roots");
  if (list_roots.HasError()) {
    throw std::invalid_argument(list_roots.error.message);
  }
  const auto root_pattern = TryReadStringField(payload, "root_pattern");
  if (root_pattern.HasError()) {
    throw std::invalid_argument(root_pattern.error.message);
  }
  const auto max_depth = TryReadIntField(payload, "max_depth");
  if (max_depth.HasError()) {
    throw std::invalid_argument(max_depth.error.message);
  }

  TreeRequestPayload out{};
  out.list_roots = list_roots.value;
  out.root_pattern = root_pattern.value;
  out.max_depth = max_depth.value;
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
  return payload.dump();
}

auto DecodeTreeResponse(std::string_view response_json) -> TreeResponsePayload {
  const json payload = ParseResponseObject(response_json);

  const auto ok = TryReadBoolField(payload, "ok");
  const auto found = TryReadBoolField(payload, "found");
  const auto error_message = TryReadStringField(payload, "error_message");
  if (ok.HasError()) {
    throw std::invalid_argument(ok.error.message);
  }
  if (!ok.value.has_value()) {
    throw std::invalid_argument("field `ok` must be a boolean.");
  }
  if (found.HasError()) {
    throw std::invalid_argument(found.error.message);
  }
  if (error_message.HasError()) {
    throw std::invalid_argument(error_message.error.message);
  }

  TreeResponsePayload out{};
  out.ok = *ok.value;
  out.found = found.value.value_or(true);
  out.error_message = error_message.value.value_or("");

  if (const auto roots_it = payload.find("roots");
      roots_it != payload.end() && !roots_it->is_null()) {
    if (!roots_it->is_array()) {
      throw std::invalid_argument("field `roots` must be a string array.");
    }
    out.roots.reserve(roots_it->size());
    for (const auto& root : *roots_it) {
      if (!root.is_string()) {
        throw std::invalid_argument("field `roots` must be a string array.");
      }
      out.roots.push_back(root.get<std::string>());
    }
  }

  if (const auto nodes_it = payload.find("nodes");
      nodes_it != payload.end() && !nodes_it->is_null()) {
    if (!nodes_it->is_array()) {
      throw std::invalid_argument("field `nodes` must be an object array.");
    }
    out.nodes.reserve(nodes_it->size());
    for (const auto& node : *nodes_it) {
      out.nodes.push_back(ParseTreeNode(node));
    }
  }

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
  }
      .dump();
}

}  // namespace tracer::transport
