// tests/transport_runtime_codec_decode_response_tests.cpp
#include "transport_runtime_codec_test_common.hpp"

namespace tracer_transport_runtime_codec_tests {
namespace {

void TestDecodeRuntimeCheckResponse(int& failures) {
  const auto response = DecodeRuntimeCheckResponse(
      R"({"ok":false,"error_message":"missing config","messages":["line1","line2"]})");
  Expect(!response.ok, "DecodeRuntimeCheckResponse ok mismatch.", failures);
  Expect(response.error_message == "missing config",
         "DecodeRuntimeCheckResponse error_message mismatch.", failures);
  Expect(response.messages.size() == 2U,
         "DecodeRuntimeCheckResponse messages size mismatch.", failures);
  Expect(response.messages[0] == "line1",
         "DecodeRuntimeCheckResponse first message mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeRuntimeCheckResponse(R"({"ok":"false"})"); },
      "field `ok` must be a boolean.",
      "DecodeRuntimeCheckResponse ok type mismatch", failures);
}

void TestDecodeResolveCliContextResponse(int& failures) {
  const auto response = DecodeResolveCliContextResponse(
      R"({"ok":true,"error_message":"","paths":{"exe_dir":"C:/bin","db_path":"C:/out/db/time_data.sqlite3","output_root":"C:/out","export_root":"C:/export","runtime_output_root":"C:/out","converter_config_toml_path":"C:/bin/config/converter.toml"},"cli_config":{"default_save_processed_output":true,"default_date_check_mode":"continuity","defaults":{"default_format":"md"},"command_defaults":{"export_format":"md","query_format":"tex","convert_date_check_mode":"full","convert_save_processed_output":false,"convert_validate_logic":true,"convert_validate_structure":false,"ingest_date_check_mode":"none","ingest_save_processed_output":true,"validate_logic_date_check_mode":"continuity"}}})");

  Expect(response.ok, "DecodeResolveCliContextResponse ok mismatch.", failures);
  Expect(response.paths.has_value(),
         "DecodeResolveCliContextResponse paths missing.", failures);
  Expect(response.cli_config.has_value(),
         "DecodeResolveCliContextResponse cli_config missing.", failures);
  Expect(response.paths->db_path == "C:/out/db/time_data.sqlite3",
         "DecodeResolveCliContextResponse db_path mismatch.", failures);
  Expect(response.paths->runtime_output_root == "C:/out",
         "DecodeResolveCliContextResponse runtime_output_root mismatch.",
         failures);
  Expect(response.cli_config->default_save_processed_output,
         "DecodeResolveCliContextResponse default_save mismatch.", failures);
  Expect(response.cli_config->default_date_check_mode.has_value() &&
             *response.cli_config->default_date_check_mode == "continuity",
         "DecodeResolveCliContextResponse default_date_check_mode mismatch.",
         failures);
  Expect(response.cli_config->command_defaults.convert_validate_logic
             .has_value() &&
             *response.cli_config->command_defaults.convert_validate_logic,
         "DecodeResolveCliContextResponse convert_validate_logic mismatch.",
         failures);

  ExpectInvalidArgument(
      [] {
        (void)DecodeResolveCliContextResponse(
            R"({"ok":true,"error_message":"","cli_config":{}})");
      },
      "field `paths` must be an object when `ok=true`.",
      "DecodeResolveCliContextResponse missing paths", failures);
  ExpectInvalidArgument(
      [] {
        (void)DecodeResolveCliContextResponse(
            R"({"ok":true,"error_message":"","paths":{"db_path":"x","runtime_output_root":"y","converter_config_toml_path":"z"},"cli_config":{"command_defaults":{"convert_validate_logic":"yes"}}})");
      },
      "field `convert_validate_logic` must be a boolean.",
      "DecodeResolveCliContextResponse bad command default type", failures);
}

void TestDecodeTreeResponse(int& failures) {
  const auto response = DecodeTreeResponse(
      R"({"ok":true,"found":false,"error_message":"","roots":["study","sleep"],"nodes":[{"name":"study","path":"study","duration_seconds":3600,"children":[{"name":"math","path":"study_math","duration_seconds":1800,"children":[]}]}]})");
  Expect(response.ok, "DecodeTreeResponse ok mismatch.", failures);
  Expect(!response.found, "DecodeTreeResponse found mismatch.", failures);
  Expect(response.roots.size() == 2U,
         "DecodeTreeResponse roots size mismatch.", failures);
  Expect(response.nodes.size() == 1U,
         "DecodeTreeResponse nodes size mismatch.", failures);
  Expect(response.nodes[0].path.has_value() &&
             *response.nodes[0].path == "study",
         "DecodeTreeResponse node path mismatch.", failures);
  Expect(response.nodes[0].duration_seconds.has_value() &&
             *response.nodes[0].duration_seconds == 3600LL,
         "DecodeTreeResponse node duration mismatch.", failures);
  Expect(response.nodes[0].children.size() == 1U,
         "DecodeTreeResponse child size mismatch.", failures);
  Expect(response.nodes[0].children[0].path.has_value() &&
             *response.nodes[0].children[0].path == "study_math",
         "DecodeTreeResponse child path mismatch.", failures);
  Expect(response.nodes[0].children[0].duration_seconds.has_value() &&
             *response.nodes[0].children[0].duration_seconds == 1800LL,
         "DecodeTreeResponse child duration mismatch.", failures);

  ExpectInvalidArgument(
      [] {
        (void)DecodeTreeResponse(
            R"({"ok":true,"error_message":"","nodes":[{"name":1}]})");
      },
      "field `name` must be a string.", "DecodeTreeResponse invalid node name",
      failures);
  ExpectInvalidArgument(
      [] { (void)DecodeTreeResponse(R"({"ok":true,"roots":[1]})"); },
      "field `roots` must be a string array.",
      "DecodeTreeResponse invalid roots type", failures);
  ExpectInvalidArgument(
      [] {
        (void)DecodeTreeResponse(
            R"({"ok":true,"nodes":[{"name":"study","duration_seconds":"bad"}]})");
      },
      "field `duration_seconds` must be an integer.",
      "DecodeTreeResponse invalid duration type", failures);
}

void TestDecodeAckAndTextResponses(int& failures) {
  const auto ack_ok =
      DecodeAckResponse(R"({"ok":true,"error_message":""})", "runtime_import");
  Expect(ack_ok.ok, "DecodeAckResponse ok mismatch.", failures);
  Expect(ack_ok.error_message.empty(),
         "DecodeAckResponse error_message mismatch.", failures);

  const auto ack_failed = DecodeAckResponse(
      R"({"ok":false,"error_message":""})", "runtime_export");
  Expect(!ack_failed.ok, "DecodeAckResponse failed ok mismatch.", failures);
  Expect(ack_failed.error_message == "Core operation failed.",
         "DecodeAckResponse failed fallback error mismatch.", failures);

  const auto text_ok = DecodeTextResponse(
      R"({"ok":true,"error_message":"","content":"query content"})",
      "runtime_query");
  Expect(text_ok.ok, "DecodeTextResponse ok mismatch.", failures);
  Expect(text_ok.error_message.empty(),
         "DecodeTextResponse error_message mismatch.", failures);
  Expect(text_ok.content == "query content",
         "DecodeTextResponse content mismatch.", failures);

  const auto text_failed = DecodeTextResponse(
      R"({"ok":false,"error_message":"","content":"partial"})", "runtime_report");
  Expect(!text_failed.ok, "DecodeTextResponse failed ok mismatch.", failures);
  Expect(text_failed.error_message == "Core operation failed.",
         "DecodeTextResponse failed fallback error mismatch.", failures);
  Expect(text_failed.content == "partial",
         "DecodeTextResponse failed content mismatch.", failures);

  ExpectInvalidArgument(
      [] {
        (void)DecodeAckResponse(R"({"error_message":"missing ok"})",
                                "runtime_validate_logic");
      },
      "runtime_validate_logic:",
      "DecodeAckResponse invalid envelope context", failures);
  ExpectInvalidArgument(
      [] { (void)DecodeTextResponse(R"({"ok":1})", "runtime_query"); },
      "runtime_query:", "DecodeTextResponse invalid envelope context",
      failures);
}

}  // namespace

auto RunDecodeResponseTests(int& failures) -> void {
  TestDecodeRuntimeCheckResponse(failures);
  TestDecodeResolveCliContextResponse(failures);
  TestDecodeTreeResponse(failures);
  TestDecodeAckAndTextResponses(failures);
}

}  // namespace tracer_transport_runtime_codec_tests
