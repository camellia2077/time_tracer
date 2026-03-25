#include "tracer/transport/fields.hpp"

#include <string>

#include "nlohmann/json.hpp"

namespace tracer::transport {

namespace {

auto WithIndefiniteArticle(std::string_view noun) -> std::string {
  if (noun.empty()) {
    return "a value";
  }
  const char kFirst = noun.front();
  const bool kStartsWithVowel =
      kFirst == 'a' || kFirst == 'e' || kFirst == 'i' || kFirst == 'o' ||
      kFirst == 'u' || kFirst == 'A' || kFirst == 'E' || kFirst == 'I' ||
      kFirst == 'O' || kFirst == 'U';
  return kStartsWithVowel ? "an " + std::string(noun)
                          : "a " + std::string(noun);
}

auto BuildTypeTransportError(std::string_view field_name,
                             std::string_view expected_type) -> TransportError {
  return MakeTransportError(TransportErrorCode::kInvalidArgument,
                            BuildTypeError(field_name, expected_type).message);
}

}  // namespace

auto BuildTypeError(std::string_view field_name, std::string_view expected_type)
    -> FieldIssue {
  return FieldIssue{
      .field_name = std::string(field_name),
      .message = "field `" + std::string(field_name) + "` must be " +
                 WithIndefiniteArticle(expected_type) + ".",
  };
}

auto FormatFieldIssue(const FieldIssue& issue) -> std::string {
  if (issue.field_name.empty()) {
    return issue.message;
  }
  return issue.field_name + ": " + issue.message;
}

auto RequireStringField(const nlohmann::json& payload,
                        std::string_view field_name) -> StringFieldResult {
  const auto kIt = payload.find(std::string(field_name));
  if (kIt == payload.end() || !kIt->is_string()) {
    return StringFieldResult{
        .value = std::nullopt,
        .error = BuildTypeTransportError(field_name, "string"),
    };
  }
  return StringFieldResult{
      .value = kIt->get<std::string>(),
      .error = TransportError{},
  };
}

auto TryReadStringField(const nlohmann::json& payload,
                        std::string_view field_name) -> StringFieldResult {
  const auto kIt = payload.find(std::string(field_name));
  if (kIt == payload.end() || kIt->is_null()) {
    return StringFieldResult{
        .value = std::nullopt,
        .error = TransportError{},
    };
  }
  if (!kIt->is_string()) {
    return StringFieldResult{
        .value = std::nullopt,
        .error = BuildTypeTransportError(field_name, "string"),
    };
  }
  return StringFieldResult{
      .value = kIt->get<std::string>(),
      .error = TransportError{},
  };
}

auto TryReadBoolField(const nlohmann::json& payload,
                      std::string_view field_name) -> BoolFieldResult {
  const auto kIt = payload.find(std::string(field_name));
  if (kIt == payload.end() || kIt->is_null()) {
    return BoolFieldResult{
        .value = std::nullopt,
        .error = TransportError{},
    };
  }
  if (!kIt->is_boolean()) {
    return BoolFieldResult{
        .value = std::nullopt,
        .error = BuildTypeTransportError(field_name, "boolean"),
    };
  }
  return BoolFieldResult{
      .value = kIt->get<bool>(),
      .error = TransportError{},
  };
}

auto TryReadIntField(const nlohmann::json& payload, std::string_view field_name)
    -> IntFieldResult {
  const auto kIt = payload.find(std::string(field_name));
  if (kIt == payload.end() || kIt->is_null()) {
    return IntFieldResult{
        .value = std::nullopt,
        .error = TransportError{},
    };
  }
  if (!kIt->is_number_integer()) {
    return IntFieldResult{
        .value = std::nullopt,
        .error = BuildTypeTransportError(field_name, "integer"),
    };
  }
  return IntFieldResult{
      .value = kIt->get<int>(),
      .error = TransportError{},
  };
}

auto TryReadIntListField(const nlohmann::json& payload,
                         std::string_view field_name) -> IntListFieldResult {
  const auto kIt = payload.find(std::string(field_name));
  if (kIt == payload.end() || kIt->is_null()) {
    return IntListFieldResult{
        .value = std::nullopt,
        .error = TransportError{},
    };
  }
  if (!kIt->is_array()) {
    return IntListFieldResult{
        .value = std::nullopt,
        .error = BuildTypeTransportError(field_name, "integer array"),
    };
  }

  std::vector<int> values;
  values.reserve(kIt->size());
  for (const auto& item : *kIt) {
    if (!item.is_number_integer()) {
      return IntListFieldResult{
          .value = std::nullopt,
          .error = BuildTypeTransportError(field_name, "integer array"),
      };
    }
    values.push_back(item.get<int>());
  }

  return IntListFieldResult{
      .value = std::move(values),
      .error = TransportError{},
  };
}

}  // namespace tracer::transport
