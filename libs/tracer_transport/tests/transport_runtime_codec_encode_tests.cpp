// tests/transport_runtime_codec_encode_tests.cpp
#include "transport_runtime_codec_test_common.hpp"

namespace tracer_transport_runtime_codec_tests {
namespace {

void TestEncodeRequestRoundTrip(int& failures) {
  {
    IngestRequestPayload request{};
    request.input_path = "test/data";
    request.date_check_mode = "none";
    request.save_processed_output = false;
    request.ingest_mode = "single_txt_replace_month";
    const auto encoded = EncodeIngestRequest(request);
    const auto decoded = DecodeIngestRequest(encoded);
    Expect(decoded.input_path == request.input_path,
           "EncodeIngestRequest round-trip input_path mismatch.", failures);
    Expect(decoded.date_check_mode == request.date_check_mode,
           "EncodeIngestRequest round-trip date_check_mode mismatch.",
           failures);
    Expect(decoded.save_processed_output == request.save_processed_output,
           "EncodeIngestRequest round-trip save_processed_output mismatch.",
           failures);
    Expect(decoded.ingest_mode == request.ingest_mode,
           "EncodeIngestRequest round-trip ingest_mode mismatch.", failures);
  }

  {
    ConvertRequestPayload request{};
    request.input_path = "test/data";
    request.date_check_mode = "none";
    request.save_processed_output = false;
    request.validate_logic = true;
    request.validate_structure = false;
    const auto encoded = EncodeConvertRequest(request);
    const auto decoded = DecodeConvertRequest(encoded);
    Expect(decoded.input_path == request.input_path,
           "EncodeConvertRequest round-trip input_path mismatch.", failures);
    Expect(decoded.date_check_mode == request.date_check_mode,
           "EncodeConvertRequest round-trip date_check_mode mismatch.",
           failures);
    Expect(decoded.save_processed_output == request.save_processed_output,
           "EncodeConvertRequest round-trip save_processed_output mismatch.",
           failures);
    Expect(decoded.validate_logic == request.validate_logic,
           "EncodeConvertRequest round-trip validate_logic mismatch.",
           failures);
    Expect(decoded.validate_structure == request.validate_structure,
           "EncodeConvertRequest round-trip validate_structure mismatch.",
           failures);
  }

  {
    ImportRequestPayload request{};
    request.processed_path = "out/data";
    const auto encoded = EncodeImportRequest(request);
    const auto decoded = DecodeImportRequest(encoded);
    Expect(decoded.processed_path == request.processed_path,
           "EncodeImportRequest round-trip processed_path mismatch.", failures);
  }

  {
    ValidateStructureRequestPayload request{};
    request.input_path = "test/data";
    const auto encoded = EncodeValidateStructureRequest(request);
    const auto decoded = DecodeValidateStructureRequest(encoded);
    Expect(decoded.input_path == request.input_path,
           "EncodeValidateStructureRequest round-trip input_path mismatch.",
           failures);
  }

  {
    ValidateLogicRequestPayload request{};
    request.input_path = "test/data";
    request.date_check_mode = "full";
    const auto encoded = EncodeValidateLogicRequest(request);
    const auto decoded = DecodeValidateLogicRequest(encoded);
    Expect(decoded.input_path == request.input_path,
           "EncodeValidateLogicRequest round-trip input_path mismatch.",
           failures);
    Expect(decoded.date_check_mode == request.date_check_mode,
           "EncodeValidateLogicRequest round-trip date_check_mode mismatch.",
           failures);
  }

  {
    QueryRequestPayload request{};
    request.action = "days";
    request.output_mode = "text";
    request.year = 2026;
    request.month = 2;
    request.root = "study";
    request.limit = 5;
    request.reverse = true;
    request.activity_score_by_duration = false;
    const auto encoded = EncodeQueryRequest(request);
    const auto decoded = DecodeQueryRequest(encoded);
    Expect(decoded.action == request.action,
           "EncodeQueryRequest round-trip action mismatch.", failures);
    Expect(decoded.output_mode == request.output_mode,
           "EncodeQueryRequest round-trip output_mode mismatch.", failures);
    Expect(decoded.year == request.year,
           "EncodeQueryRequest round-trip year mismatch.", failures);
    Expect(decoded.root == request.root,
           "EncodeQueryRequest round-trip root mismatch.", failures);
    Expect(decoded.reverse == request.reverse,
           "EncodeQueryRequest round-trip reverse mismatch.", failures);
    Expect(decoded.activity_score_by_duration ==
               request.activity_score_by_duration,
           "EncodeQueryRequest round-trip score flag mismatch.", failures);
  }

  {
    ReportRequestPayload request{};
    request.type = "month";
    request.argument = "2026-02";
    request.format = "markdown";
    const auto encoded = EncodeReportRequest(request);
    const auto decoded = DecodeReportRequest(encoded);
    Expect(decoded.type == request.type,
           "EncodeReportRequest round-trip type mismatch.", failures);
    Expect(decoded.argument == request.argument,
           "EncodeReportRequest round-trip argument mismatch.", failures);
    Expect(decoded.format == request.format,
           "EncodeReportRequest round-trip format mismatch.", failures);
  }

  {
    ReportBatchRequestPayload request{};
    request.days_list = {7, 14, 30};
    request.format = "md";
    const auto encoded = EncodeReportBatchRequest(request);
    const auto decoded = DecodeReportBatchRequest(encoded);
    Expect(decoded.days_list == request.days_list,
           "EncodeReportBatchRequest round-trip days_list mismatch.", failures);
    Expect(decoded.format == request.format,
           "EncodeReportBatchRequest round-trip format mismatch.", failures);
  }

  {
    ExportRequestPayload request{};
    request.type = "all-month";
    request.argument = "2026-02";
    request.format = "markdown";
    request.recent_days_list = std::vector<int>{7, 14};
    const auto encoded = EncodeExportRequest(request);
    const auto decoded = DecodeExportRequest(encoded);
    Expect(decoded.type == request.type,
           "EncodeExportRequest round-trip type mismatch.", failures);
    Expect(decoded.argument == request.argument,
           "EncodeExportRequest round-trip argument mismatch.", failures);
    Expect(decoded.recent_days_list == request.recent_days_list,
           "EncodeExportRequest round-trip recent_days_list mismatch.",
           failures);
  }

  {
    TreeRequestPayload request{};
    request.list_roots = false;
    request.root_pattern = "study";
    request.max_depth = 2;
    request.period = "recent";
    request.period_argument = "7";
    request.root = "study";
    const auto encoded = EncodeTreeRequest(request);
    const auto decoded = DecodeTreeRequest(encoded);
    Expect(decoded.list_roots == request.list_roots,
           "EncodeTreeRequest round-trip list_roots mismatch.", failures);
    Expect(decoded.root_pattern == request.root_pattern,
           "EncodeTreeRequest round-trip root_pattern mismatch.", failures);
    Expect(decoded.max_depth == request.max_depth,
           "EncodeTreeRequest round-trip max_depth mismatch.", failures);
    Expect(decoded.period == request.period,
           "EncodeTreeRequest round-trip period mismatch.", failures);
    Expect(decoded.period_argument == request.period_argument,
           "EncodeTreeRequest round-trip period_argument mismatch.", failures);
    Expect(decoded.root == request.root,
           "EncodeTreeRequest round-trip root mismatch.", failures);
  }
}

void TestEncodeResponses(int& failures) {
  const std::string ingest_json =
      EncodeIngestResponse(IngestResponsePayload{.ok = true, .error_message = ""});
  const json ingest = json::parse(ingest_json);
  Expect(ingest.value("ok", false), "EncodeIngestResponse ok mismatch.",
         failures);
  Expect(ingest.value("error_message", std::string("x")).empty(),
         "EncodeIngestResponse error_message mismatch.", failures);

  const std::string query_json = EncodeQueryResponse(
      QueryResponsePayload{.ok = true,
                           .error_message = "",
                           .content = "2026\nTotal: 1\n"});
  const json query = json::parse(query_json);
  Expect(query.value("ok", false), "EncodeQueryResponse ok mismatch.",
         failures);
  Expect(query.value("content", std::string{}) == "2026\nTotal: 1\n",
         "EncodeQueryResponse content mismatch.", failures);

  const std::string report_json = EncodeReportResponse(
      ReportResponsePayload{.ok = true,
                            .error_message = "",
                            .content = "## Monthly Summary"});
  const json report = json::parse(report_json);
  Expect(report.value("content", std::string{}) == "## Monthly Summary",
         "EncodeReportResponse content mismatch.", failures);

  const std::string batch_json = EncodeReportBatchResponse(
      ReportBatchResponsePayload{.ok = false,
                                 .error_message = "failed",
                                 .content = ""});
  const json batch = json::parse(batch_json);
  Expect(!batch.value("ok", true), "EncodeReportBatchResponse ok mismatch.",
         failures);
  Expect(batch.value("error_message", std::string{}) == "failed",
         "EncodeReportBatchResponse error mismatch.", failures);

  const std::string export_json = EncodeExportResponse(
      ExportResponsePayload{.ok = false, .error_message = "export failed"});
  const json export_payload = json::parse(export_json);
  Expect(!export_payload.value("ok", true), "EncodeExportResponse ok mismatch.",
         failures);
  Expect(export_payload.value("error_message", std::string{}) ==
             "export failed",
         "EncodeExportResponse error mismatch.", failures);

  CapabilitiesResponsePayload capabilities{};
  capabilities.abi.name = "tracer_core_c";
  capabilities.abi.version = 1;
  capabilities.features.runtime_log_callback = true;
  capabilities.features.runtime_diagnostics_callback = true;
  capabilities.features.runtime_crypto_progress_callback = true;
  capabilities.features.runtime_ingest_json = true;
  capabilities.features.runtime_convert_json = true;
  capabilities.features.runtime_import_json = true;
  capabilities.features.runtime_validate_structure_json = true;
  capabilities.features.runtime_validate_logic_json = true;
  capabilities.features.runtime_query_json = true;
  capabilities.features.runtime_report_json = true;
  capabilities.features.runtime_report_batch_json = true;
  capabilities.features.runtime_export_json = true;
  capabilities.features.runtime_tree_json = true;
  capabilities.features.processed_json_io = true;
  capabilities.features.report_markdown = true;
  capabilities.features.report_latex = false;
  capabilities.features.report_typst = false;
  const json capabilities_json =
      json::parse(EncodeCapabilitiesResponse(capabilities));
  Expect(capabilities_json["abi"].value("name", std::string{}) ==
             "tracer_core_c",
         "EncodeCapabilitiesResponse abi.name mismatch.", failures);
  Expect(capabilities_json["abi"].value("version", 0) == 1,
         "EncodeCapabilitiesResponse abi.version mismatch.", failures);
  Expect(capabilities_json["features"].value("runtime_ingest_json", false),
         "EncodeCapabilitiesResponse features.runtime_ingest_json mismatch.",
         failures);
  Expect(capabilities_json["features"].value("runtime_log_callback", false),
         "EncodeCapabilitiesResponse features.runtime_log_callback mismatch.",
         failures);
  Expect(capabilities_json["features"].value("runtime_diagnostics_callback",
                                             false),
         "EncodeCapabilitiesResponse features.runtime_diagnostics_callback "
         "mismatch.",
         failures);
  Expect(capabilities_json["features"].value("runtime_crypto_progress_callback",
                                             false),
         "EncodeCapabilitiesResponse "
         "features.runtime_crypto_progress_callback mismatch.",
         failures);
  Expect(!capabilities_json["features"].value("report_latex", true),
         "EncodeCapabilitiesResponse features.report_latex mismatch.",
         failures);
}

void TestEncodeTreeResponse(int& failures) {
  TreeResponsePayload payload{};
  payload.ok = true;
  payload.found = true;
  payload.error_message = "";
  payload.roots = {"study", "sleep"};

  ProjectTreeNodePayload node{};
  node.name = "study";
  node.path = "study";
  node.duration_seconds = 3600;
  ProjectTreeNodePayload child{};
  child.name = "math";
  child.path = "study_math";
  child.duration_seconds = 1800;
  node.children.push_back(child);
  payload.nodes.push_back(node);

  const std::string tree_json = EncodeTreeResponse(payload);
  const json tree = json::parse(tree_json);

  Expect(tree.value("ok", false), "EncodeTreeResponse ok mismatch.", failures);
  Expect(tree.value("found", false), "EncodeTreeResponse found mismatch.",
         failures);
  Expect(tree.contains("roots") && tree["roots"].is_array() &&
             tree["roots"].size() == 2U,
         "EncodeTreeResponse roots mismatch.", failures);
  Expect(tree.contains("nodes") && tree["nodes"].is_array() &&
             tree["nodes"].size() == 1U,
         "EncodeTreeResponse nodes mismatch.", failures);
  Expect(tree["nodes"][0].value("name", std::string{}) == "study",
         "EncodeTreeResponse root node name mismatch.", failures);
  Expect(tree["nodes"][0].value("path", std::string{}) == "study",
         "EncodeTreeResponse root node path mismatch.", failures);
  Expect(tree["nodes"][0].value("duration_seconds", -1LL) == 3600LL,
         "EncodeTreeResponse root node duration mismatch.", failures);
  Expect(tree["nodes"][0].contains("children") &&
             tree["nodes"][0]["children"].is_array() &&
             tree["nodes"][0]["children"].size() == 1U,
         "EncodeTreeResponse child node mismatch.", failures);
  Expect(tree["nodes"][0]["children"][0].value("path", std::string{}) ==
             "study_math",
         "EncodeTreeResponse child node path mismatch.", failures);
  Expect(tree["nodes"][0]["children"][0].value("duration_seconds", -1LL) ==
             1800LL,
         "EncodeTreeResponse child node duration mismatch.", failures);
}

}  // namespace

auto RunEncodeTests(int& failures) -> void {
  TestEncodeRequestRoundTrip(failures);
  TestEncodeResponses(failures);
  TestEncodeTreeResponse(failures);
}

}  // namespace tracer_transport_runtime_codec_tests
