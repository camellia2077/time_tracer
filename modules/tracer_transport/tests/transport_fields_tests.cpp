#include <iostream>
#include <string_view>

#include "nlohmann/json.hpp"
#include "tracer/transport/fields.hpp"

namespace {

using nlohmann::json;
using tracer::transport::BuildTypeError;
using tracer::transport::FormatFieldIssue;
using tracer::transport::RequireStringField;
using tracer::transport::TransportErrorCode;
using tracer::transport::TryReadBoolField;
using tracer::transport::TryReadIntField;
using tracer::transport::TryReadIntListField;
using tracer::transport::TryReadStringField;

auto Contains(std::string_view text, std::string_view pattern) -> bool {
  return text.find(pattern) != std::string_view::npos;
}

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestTypeErrorFormatting(int& failures) {
  const auto issue = BuildTypeError("days_list", "integer array");
  Expect(issue.message == "field `days_list` must be an integer array.",
         "BuildTypeError should produce centralized type error message.",
         failures);
  Expect(FormatFieldIssue(issue) ==
             "days_list: field `days_list` must be an integer array.",
         "FormatFieldIssue output mismatch.", failures);
}

void TestRequireStringField(int& failures) {
  const json payload = {
      {"action", "years"},
  };
  const auto ok = RequireStringField(payload, "action");
  Expect(!ok.HasError(), "RequireStringField should succeed on string field.",
         failures);
  Expect(ok.value.has_value() && *ok.value == "years",
         "RequireStringField value mismatch.", failures);

  const auto missing = RequireStringField(payload, "type");
  Expect(missing.HasError(),
         "RequireStringField should fail on missing field.", failures);
  Expect(missing.error.code == TransportErrorCode::kInvalidArgument,
         "RequireStringField missing should use invalid-argument error code.",
         failures);
  Expect(missing.error.message == "field `type` must be a string.",
         "RequireStringField missing message mismatch.", failures);
}

void TestTryReadStringField(int& failures) {
  const json payload = {
      {"remark", "hello"},
      {"project", nullptr},
      {"status", 1},
  };

  const auto found = TryReadStringField(payload, "remark");
  Expect(!found.HasError(), "TryReadStringField should succeed on string.",
         failures);
  Expect(found.value.has_value() && *found.value == "hello",
         "TryReadStringField value mismatch.", failures);

  const auto null_value = TryReadStringField(payload, "project");
  Expect(!null_value.HasError(),
         "TryReadStringField should allow null as absent.", failures);
  Expect(!null_value.value.has_value(),
         "TryReadStringField null should return nullopt.", failures);

  const auto missing = TryReadStringField(payload, "day_remark");
  Expect(!missing.HasError(),
         "TryReadStringField should allow missing field.", failures);
  Expect(!missing.value.has_value(),
         "TryReadStringField missing should return nullopt.", failures);

  const auto wrong_type = TryReadStringField(payload, "status");
  Expect(wrong_type.HasError(),
         "TryReadStringField should fail on non-string field.", failures);
  Expect(wrong_type.error.message == "field `status` must be a string.",
         "TryReadStringField wrong-type message mismatch.", failures);
}

void TestTryReadBoolField(int& failures) {
  const json payload = {
      {"overnight", true},
      {"reverse", "no"},
  };

  const auto ok = TryReadBoolField(payload, "overnight");
  Expect(!ok.HasError(), "TryReadBoolField should succeed on bool.", failures);
  Expect(ok.value.has_value() && *ok.value,
         "TryReadBoolField bool value mismatch.", failures);

  const auto missing = TryReadBoolField(payload, "activity_score_by_duration");
  Expect(!missing.HasError(), "TryReadBoolField missing should be allowed.",
         failures);
  Expect(!missing.value.has_value(),
         "TryReadBoolField missing should return nullopt.", failures);

  const auto wrong_type = TryReadBoolField(payload, "reverse");
  Expect(wrong_type.HasError(),
         "TryReadBoolField should fail on wrong type.", failures);
  Expect(wrong_type.error.message == "field `reverse` must be a boolean.",
         "TryReadBoolField wrong-type message mismatch.", failures);
}

void TestTryReadIntField(int& failures) {
  const json payload = {
      {"year", 2026},
      {"month", 1.5},
  };

  const auto ok = TryReadIntField(payload, "year");
  Expect(!ok.HasError(), "TryReadIntField should succeed on integer.", failures);
  Expect(ok.value.has_value() && *ok.value == 2026,
         "TryReadIntField integer value mismatch.", failures);

  const auto wrong_type = TryReadIntField(payload, "month");
  Expect(wrong_type.HasError(),
         "TryReadIntField should fail on non-integer number.", failures);
  Expect(wrong_type.error.message == "field `month` must be an integer.",
         "TryReadIntField wrong-type message mismatch.", failures);
}

void TestTryReadIntListField(int& failures) {
  const json payload = {
      {"days_list", json::array({7, 14, 30})},
      {"bad_list", json::array({7, "x", 30})},
      {"bad_shape", "not-array"},
  };

  const auto ok = TryReadIntListField(payload, "days_list");
  Expect(!ok.HasError(), "TryReadIntListField should succeed on int array.",
         failures);
  Expect(ok.value.has_value() && ok.value->size() == 3U,
         "TryReadIntListField array size mismatch.", failures);

  const auto bad_item = TryReadIntListField(payload, "bad_list");
  Expect(bad_item.HasError(),
         "TryReadIntListField should fail when array contains non-integer.",
         failures);
  Expect(Contains(bad_item.error.message, "integer array"),
         "TryReadIntListField bad-item message should mention integer array.",
         failures);

  const auto bad_shape = TryReadIntListField(payload, "bad_shape");
  Expect(bad_shape.HasError(),
         "TryReadIntListField should fail on non-array field.", failures);
  Expect(bad_shape.error.message ==
             "field `bad_shape` must be an integer array.",
         "TryReadIntListField bad-shape message mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestTypeErrorFormatting(failures);
  TestRequireStringField(failures);
  TestTryReadStringField(failures);
  TestTryReadBoolField(failures);
  TestTryReadIntField(failures);
  TestTryReadIntListField(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_fields_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_fields_tests failures: " << failures
            << '\n';
  return 1;
}
