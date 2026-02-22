#include <iostream>
#include <string>
#include <string_view>

#include "tracer/transport/envelope.hpp"

namespace {

using tracer::transport::BuildResponseEnvelope;
using tracer::transport::ParseResponseEnvelope;
using tracer::transport::SerializeResponseEnvelope;
using tracer::transport::TransportErrorCode;

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

void TestParseSuccess(int& failures) {
  const auto parsed = ParseResponseEnvelope(
      R"({"ok":true,"error_message":"","content":"ingest completed"})",
      "nativeIngest");
  Expect(!parsed.HasError(), "Parse success should not return error.", failures);
  Expect(parsed.envelope.ok, "Parsed envelope `ok` should be true.", failures);
  Expect(parsed.envelope.error_message.empty(),
         "Parsed envelope `error_message` should be empty.", failures);
  Expect(parsed.envelope.content == "ingest completed",
         "Parsed envelope `content` mismatch.", failures);
}

void TestParseMissingOptionalDefaults(int& failures) {
  const auto parsed = ParseResponseEnvelope(R"({"ok":false})", "nativeQuery");
  Expect(!parsed.HasError(),
         "Missing optional fields should not fail parsing.", failures);
  Expect(!parsed.envelope.ok, "Parsed envelope `ok` should be false.", failures);
  Expect(parsed.envelope.error_message.empty(),
         "Missing `error_message` should default to empty string.", failures);
  Expect(parsed.envelope.content.empty(),
         "Missing `content` should default to empty string.", failures);
}

void TestParseOptionalTypeMismatchDefaults(int& failures) {
  const auto parsed = ParseResponseEnvelope(
      R"({"ok":true,"error_message":123,"content":["x"]})", "nativeReport");
  Expect(!parsed.HasError(),
         "Type mismatch on optional fields should not fail parsing.", failures);
  Expect(parsed.envelope.ok, "Parsed envelope `ok` should be true.", failures);
  Expect(parsed.envelope.error_message.empty(),
         "Non-string `error_message` should default to empty string.", failures);
  Expect(parsed.envelope.content.empty(),
         "Non-string `content` should default to empty string.", failures);
}

void TestParseRejectsMissingOrInvalidOk(int& failures) {
  const auto missing_ok = ParseResponseEnvelope(
      R"({"error_message":"","content":"x"})", "nativeValidateLogic");
  Expect(missing_ok.HasError(), "Missing `ok` should fail parsing.", failures);
  Expect(missing_ok.error.code == TransportErrorCode::kValidationFailure,
         "Missing `ok` should be a validation failure.", failures);
  Expect(Contains(missing_ok.error.message, "nativeValidateLogic:"),
         "Validation error should include context prefix.", failures);
  Expect(Contains(missing_ok.error.message, "missing boolean `ok`"),
         "Validation error should mention `ok` field.", failures);

  const auto wrong_ok =
      ParseResponseEnvelope(R"({"ok":"true"})", "nativeValidateLogic");
  Expect(wrong_ok.HasError(), "Non-boolean `ok` should fail parsing.", failures);
  Expect(wrong_ok.error.code == TransportErrorCode::kValidationFailure,
         "Non-boolean `ok` should be a validation failure.", failures);
}

void TestParseEmptyResponse(int& failures) {
  const auto parsed = ParseResponseEnvelope("", "nativeQuery");
  Expect(parsed.HasError(), "Empty response should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Empty response should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "nativeQuery: empty core response."),
         "Empty response error message mismatch.", failures);
}

void TestParseInvalidJson(int& failures) {
  const auto parsed = ParseResponseEnvelope("{not-json", "nativeReport");
  Expect(parsed.HasError(), "Invalid JSON should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Invalid JSON should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "nativeReport:"),
         "Invalid JSON error should include context prefix.", failures);
}

void TestParseNonObjectJson(int& failures) {
  const auto parsed = ParseResponseEnvelope("[]", "nativeTree");
  Expect(parsed.HasError(), "Non-object JSON should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Non-object JSON should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "non-object JSON core response."),
         "Non-object JSON error message mismatch.", failures);
}

void TestSerializeRoundTrip(int& failures) {
  const auto envelope = BuildResponseEnvelope(
      true, "line 1\nline 2 \"quoted\"", "content payload");
  const std::string serialized = SerializeResponseEnvelope(envelope);
  const auto parsed = ParseResponseEnvelope(serialized, "roundtrip");
  Expect(!parsed.HasError(), "Serialized envelope should parse back.", failures);
  Expect(parsed.envelope.ok == envelope.ok, "Roundtrip `ok` mismatch.",
         failures);
  Expect(parsed.envelope.error_message == envelope.error_message,
         "Roundtrip `error_message` mismatch.", failures);
  Expect(parsed.envelope.content == envelope.content,
         "Roundtrip `content` mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestParseSuccess(failures);
  TestParseMissingOptionalDefaults(failures);
  TestParseOptionalTypeMismatchDefaults(failures);
  TestParseRejectsMissingOrInvalidOk(failures);
  TestParseEmptyResponse(failures);
  TestParseInvalidJson(failures);
  TestParseNonObjectJson(failures);
  TestSerializeRoundTrip(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_tests failures: " << failures << '\n';
  return 1;
}
