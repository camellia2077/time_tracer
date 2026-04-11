#include <iostream>
#include <string>
#include <string_view>

#include "tracer/transport/envelope.hpp"

namespace {

using tracer::transport::BuildResponseEnvelope;
using tracer::transport::ParseResponseEnvelope;
using tracer::transport::ResponseEnvelopeParseArgs;
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
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json =
          R"({"ok":true,"error_message":"","content":"ingest completed"})",
      .context = "nativeIngest",
  });
  Expect(!parsed.HasError(), "Parse success should not return error.",
         failures);
  Expect(parsed.envelope.ok, "Parsed envelope `ok` should be true.", failures);
  Expect(parsed.envelope.error_message.empty(),
         "Parsed envelope `error_message` should be empty.", failures);
  Expect(parsed.envelope.content == "ingest completed",
         "Parsed envelope `content` mismatch.", failures);
}

void TestParseMissingOptionalDefaults(int& failures) {
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = R"({"ok":false})",
      .context = "nativeQuery",
  });
  Expect(!parsed.HasError(), "Missing optional fields should not fail parsing.",
         failures);
  Expect(!parsed.envelope.ok, "Parsed envelope `ok` should be false.",
         failures);
  Expect(parsed.envelope.error_message.empty(),
         "Missing `error_message` should default to empty string.", failures);
  Expect(parsed.envelope.content.empty(),
         "Missing `content` should default to empty string.", failures);
  Expect(!parsed.envelope.report_hash_sha256.has_value(),
         "Missing `report_hash_sha256` should default to nullopt.", failures);
  Expect(!parsed.envelope.report_window_metadata.has_value(),
         "Missing report window metadata should default to nullopt.",
         failures);
}

void TestParseOptionalTypeMismatchDefaults(int& failures) {
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = R"({"ok":true,"error_message":123,"content":["x"]})",
      .context = "nativeReport",
  });
  Expect(!parsed.HasError(),
         "Type mismatch on optional fields should not fail parsing.", failures);
  Expect(parsed.envelope.ok, "Parsed envelope `ok` should be true.", failures);
  Expect(parsed.envelope.error_message.empty(),
         "Non-string `error_message` should default to empty string.",
         failures);
  Expect(parsed.envelope.content.empty(),
         "Non-string `content` should default to empty string.", failures);
}

void TestParseRejectsMissingOrInvalidOk(int& failures) {
  const auto missing_ok = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = R"({"error_message":"","content":"x"})",
      .context = "nativeValidateLogic",
  });
  Expect(missing_ok.HasError(), "Missing `ok` should fail parsing.", failures);
  Expect(missing_ok.error.code == TransportErrorCode::kValidationFailure,
         "Missing `ok` should be a validation failure.", failures);
  Expect(Contains(missing_ok.error.message, "nativeValidateLogic:"),
         "Validation error should include context prefix.", failures);
  Expect(Contains(missing_ok.error.message, "missing boolean `ok`"),
         "Validation error should mention `ok` field.", failures);

  const auto wrong_ok = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = R"({"ok":"true"})",
      .context = "nativeValidateLogic",
  });
  Expect(wrong_ok.HasError(), "Non-boolean `ok` should fail parsing.",
         failures);
  Expect(wrong_ok.error.code == TransportErrorCode::kValidationFailure,
         "Non-boolean `ok` should be a validation failure.", failures);
}

void TestParseEmptyResponse(int& failures) {
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = "",
      .context = "nativeQuery",
  });
  Expect(parsed.HasError(), "Empty response should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Empty response should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "nativeQuery: empty core response."),
         "Empty response error message mismatch.", failures);
}

void TestParseInvalidJson(int& failures) {
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = "{not-json",
      .context = "nativeReport",
  });
  Expect(parsed.HasError(), "Invalid JSON should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Invalid JSON should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "nativeReport:"),
         "Invalid JSON error should include context prefix.", failures);
}

void TestParseNonObjectJson(int& failures) {
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = "[]",
      .context = "nativeTree",
  });
  Expect(parsed.HasError(), "Non-object JSON should fail parsing.", failures);
  Expect(parsed.error.code == TransportErrorCode::kParseFailure,
         "Non-object JSON should be parse failure.", failures);
  Expect(Contains(parsed.error.message, "non-object JSON core response."),
         "Non-object JSON error message mismatch.", failures);
}

void TestSerializeRoundTrip(int& failures) {
  const auto envelope = BuildResponseEnvelope(true, "line 1\nline 2 \"quoted\"",
                                              "content payload");
  const std::string serialized = SerializeResponseEnvelope(envelope);
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = serialized,
      .context = "roundtrip",
  });
  Expect(!parsed.HasError(), "Serialized envelope should parse back.",
         failures);
  Expect(parsed.envelope.ok == envelope.ok, "Roundtrip `ok` mismatch.",
         failures);
  Expect(parsed.envelope.error_message == envelope.error_message,
         "Roundtrip `error_message` mismatch.", failures);
  Expect(parsed.envelope.content == envelope.content,
         "Roundtrip `content` mismatch.", failures);
  Expect(!parsed.envelope.report_hash_sha256.has_value(),
         "Roundtrip missing hash should keep nullopt.", failures);
  Expect(!parsed.envelope.report_window_metadata.has_value(),
         "Roundtrip missing window metadata should keep nullopt.", failures);
}

void TestSerializeRoundTripWithHash(int& failures) {
  auto envelope = BuildResponseEnvelope(true, "", "report body");
  envelope.report_hash_sha256 =
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
  const std::string serialized = SerializeResponseEnvelope(envelope);
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = serialized,
      .context = "roundtrip_with_hash",
  });
  Expect(!parsed.HasError(), "Serialized envelope with hash should parse back.",
         failures);
  Expect(parsed.envelope.report_hash_sha256 == envelope.report_hash_sha256,
         "Roundtrip `report_hash_sha256` mismatch.", failures);
}

void TestSerializeRoundTripWithErrorContract(int& failures) {
  const auto envelope = BuildResponseEnvelope(
      false, "", "partial payload", "runtime.invalid_request", "runtime",
      {"check request", "check contract"});
  const std::string serialized = SerializeResponseEnvelope(envelope);
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = serialized,
      .context = "roundtrip_with_error_contract",
  });
  Expect(!parsed.HasError(),
         "Serialized envelope with error contract should parse back.",
         failures);
  Expect(!parsed.envelope.ok, "Roundtrip error envelope `ok` mismatch.",
         failures);
  Expect(parsed.envelope.error_code == "runtime.invalid_request",
         "Roundtrip `error_code` mismatch.", failures);
  Expect(parsed.envelope.error_category == "runtime",
         "Roundtrip `error_category` mismatch.", failures);
  Expect(parsed.envelope.hints.size() == 2U,
         "Roundtrip `hints` size mismatch.", failures);
  Expect(parsed.envelope.hints[0] == "check request",
         "Roundtrip first hint mismatch.", failures);
  Expect(parsed.envelope.content == "partial payload",
         "Roundtrip error envelope content mismatch.", failures);
}

void TestSerializeRoundTripWithWindowMetadata(int& failures) {
  auto envelope = BuildResponseEnvelope(true, "", "report body");
  envelope.report_window_metadata =
      tracer::transport::ReportWindowMetadataPayload{
          .has_records = false,
          .matched_day_count = 0,
          .matched_record_count = 0,
          .start_date = "2024-12-01",
          .end_date = "2024-12-31",
          .requested_days = 31,
      };
  const std::string serialized = SerializeResponseEnvelope(envelope);
  const auto parsed = ParseResponseEnvelope(ResponseEnvelopeParseArgs{
      .response_json = serialized,
      .context = "roundtrip_with_window_metadata",
  });
  Expect(!parsed.HasError(),
         "Serialized envelope with window metadata should parse back.",
         failures);
  Expect(parsed.envelope.report_window_metadata.has_value(),
         "Roundtrip window metadata should be present.", failures);
  Expect(parsed.envelope.report_window_metadata->requested_days == 31,
         "Roundtrip requested_days mismatch.", failures);
  Expect(parsed.envelope.report_window_metadata->start_date == "2024-12-01",
         "Roundtrip start_date mismatch.", failures);
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
  TestSerializeRoundTripWithHash(failures);
  TestSerializeRoundTripWithErrorContract(failures);
  TestSerializeRoundTripWithWindowMetadata(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_tests failures: " << failures << '\n';
  return 1;
}
