#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "nlohmann/json_fwd.hpp"
#include "tracer/transport/errors.hpp"

namespace tracer::transport {

struct FieldIssue {
  std::string field_name;
  std::string message;
};

struct StringFieldResult {
  std::optional<std::string> value;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

struct BoolFieldResult {
  std::optional<bool> value;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

struct IntFieldResult {
  std::optional<int> value;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

struct IntListFieldResult {
  std::optional<std::vector<int>> value;
  TransportError error;

  [[nodiscard]] auto HasError() const -> bool { return error.HasError(); }
};

[[nodiscard]] auto BuildTypeError(std::string_view field_name,
                                  std::string_view expected_type)
    -> FieldIssue;

[[nodiscard]] auto FormatFieldIssue(const FieldIssue& issue) -> std::string;

[[nodiscard]] auto RequireStringField(const nlohmann::json& payload,
                                      std::string_view field_name)
    -> StringFieldResult;

[[nodiscard]] auto TryReadStringField(const nlohmann::json& payload,
                                      std::string_view field_name)
    -> StringFieldResult;

[[nodiscard]] auto TryReadBoolField(const nlohmann::json& payload,
                                    std::string_view field_name)
    -> BoolFieldResult;

[[nodiscard]] auto TryReadIntField(const nlohmann::json& payload,
                                   std::string_view field_name)
    -> IntFieldResult;

[[nodiscard]] auto TryReadIntListField(const nlohmann::json& payload,
                                       std::string_view field_name)
    -> IntListFieldResult;

}  // namespace tracer::transport
