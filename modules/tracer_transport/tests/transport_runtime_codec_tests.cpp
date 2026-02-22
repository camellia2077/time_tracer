#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "nlohmann/json.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace {

using nlohmann::json;
using tracer::transport::CapabilitiesResponsePayload;
using tracer::transport::ConvertRequestPayload;
using tracer::transport::DecodeExportRequest;
using tracer::transport::DecodeConvertRequest;
using tracer::transport::DecodeIngestRequest;
using tracer::transport::DecodeImportRequest;
using tracer::transport::DecodeQueryRequest;
using tracer::transport::DecodeResolveCliContextResponse;
using tracer::transport::DecodeReportBatchRequest;
using tracer::transport::DecodeReportRequest;
using tracer::transport::DecodeRuntimeCheckResponse;
using tracer::transport::DecodeTreeResponse;
using tracer::transport::DecodeTreeRequest;
using tracer::transport::DecodeValidateLogicRequest;
using tracer::transport::DecodeValidateStructureRequest;
using tracer::transport::EncodeCapabilitiesResponse;
using tracer::transport::EncodeConvertRequest;
using tracer::transport::EncodeExportRequest;
using tracer::transport::EncodeExportResponse;
using tracer::transport::EncodeIngestRequest;
using tracer::transport::EncodeIngestResponse;
using tracer::transport::EncodeImportRequest;
using tracer::transport::EncodeQueryRequest;
using tracer::transport::EncodeQueryResponse;
using tracer::transport::EncodeReportBatchRequest;
using tracer::transport::EncodeReportBatchResponse;
using tracer::transport::EncodeReportRequest;
using tracer::transport::EncodeReportResponse;
using tracer::transport::EncodeTreeRequest;
using tracer::transport::EncodeTreeResponse;
using tracer::transport::EncodeValidateLogicRequest;
using tracer::transport::EncodeValidateStructureRequest;
using tracer::transport::ExportRequestPayload;
using tracer::transport::ExportResponsePayload;
using tracer::transport::IngestRequestPayload;
using tracer::transport::IngestResponsePayload;
using tracer::transport::ImportRequestPayload;
using tracer::transport::ProjectTreeNodePayload;
using tracer::transport::QueryRequestPayload;
using tracer::transport::QueryResponsePayload;
using tracer::transport::ReportBatchRequestPayload;
using tracer::transport::ReportBatchResponsePayload;
using tracer::transport::ReportRequestPayload;
using tracer::transport::ReportResponsePayload;
using tracer::transport::TreeRequestPayload;
using tracer::transport::TreeResponsePayload;
using tracer::transport::ValidateLogicRequestPayload;
using tracer::transport::ValidateStructureRequestPayload;

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

template <typename Fn>
void ExpectInvalidArgument(Fn&& fn, std::string_view contains_message,
                           std::string_view test_name, int& failures) {
  try {
    fn();
  } catch (const std::invalid_argument& error) {
    if (!Contains(error.what(), contains_message)) {
      ++failures;
      std::cerr << "[FAIL] " << test_name
                << ": unexpected invalid_argument message: " << error.what()
                << '\n';
    }
    return;
  } catch (const std::exception& error) {
    ++failures;
    std::cerr << "[FAIL] " << test_name
              << ": expected invalid_argument, got exception: " << error.what()
              << '\n';
    return;
  }

  ++failures;
  std::cerr << "[FAIL] " << test_name << ": expected invalid_argument.\n";
}

void TestDecodeIngestRequest(int& failures) {
  const auto request = DecodeIngestRequest(
      R"({"input_path":"test/data","date_check_mode":"none","save_processed_output":true,"ingest_mode":"single_txt_replace_month"})");
  Expect(request.input_path == "test/data",
         "DecodeIngestRequest input_path mismatch.", failures);
  Expect(request.date_check_mode.has_value() &&
             *request.date_check_mode == "none",
         "DecodeIngestRequest date_check_mode mismatch.", failures);
  Expect(request.save_processed_output.has_value() &&
             *request.save_processed_output,
         "DecodeIngestRequest save_processed_output mismatch.", failures);
  Expect(request.ingest_mode.has_value() &&
             *request.ingest_mode == "single_txt_replace_month",
         "DecodeIngestRequest ingest_mode mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeIngestRequest(R"({"date_check_mode":"none"})"); },
      "field `input_path` must be a string.", "DecodeIngestRequest missing",
      failures);
}

void TestDecodeQueryRequest(int& failures) {
  const auto request = DecodeQueryRequest(
      R"({"action":"days_duration","output_mode":"semantic_json","year":2026,"month":1,"from_date":"2026-01-01","to_date":"2026-01-31","remark":"x","day_remark":"y","project":"study","root":"study","exercise":1,"status":0,"overnight":false,"reverse":true,"limit":7,"top_n":3,"lookback_days":14,"activity_prefix":"st","activity_score_by_duration":true,"tree_period":"recent","tree_period_argument":"7","tree_max_depth":2})");

  Expect(request.action == "days_duration", "DecodeQueryRequest action mismatch.",
         failures);
  Expect(request.output_mode.has_value() &&
             *request.output_mode == "semantic_json",
         "DecodeQueryRequest output_mode mismatch.", failures);
  Expect(request.year.has_value() && *request.year == 2026,
         "DecodeQueryRequest year mismatch.", failures);
  Expect(request.month.has_value() && *request.month == 1,
         "DecodeQueryRequest month mismatch.", failures);
  Expect(request.from_date.has_value() && *request.from_date == "2026-01-01",
         "DecodeQueryRequest from_date mismatch.", failures);
  Expect(request.reverse.has_value() && *request.reverse,
         "DecodeQueryRequest reverse mismatch.", failures);
  Expect(request.activity_score_by_duration.has_value() &&
             *request.activity_score_by_duration,
         "DecodeQueryRequest score flag mismatch.", failures);
  Expect(request.root.has_value() && *request.root == "study",
         "DecodeQueryRequest root mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeQueryRequest(R"({"action":1})"); },
      "field `action` must be a string.", "DecodeQueryRequest bad action type",
      failures);
  ExpectInvalidArgument(
      [] { (void)DecodeQueryRequest(R"({"action":"days","output_mode":1})"); },
      "field `output_mode` must be a string.",
      "DecodeQueryRequest bad output_mode type", failures);
}

void TestDecodeWorkflowRequests(int& failures) {
  const auto convert = DecodeConvertRequest(
      R"({"input_path":"test/data","date_check_mode":"none","save_processed_output":true,"validate_logic":false,"validate_structure":true})");
  Expect(convert.input_path == "test/data",
         "DecodeConvertRequest input_path mismatch.", failures);
  Expect(convert.date_check_mode.has_value() &&
             *convert.date_check_mode == "none",
         "DecodeConvertRequest date_check_mode mismatch.", failures);
  Expect(convert.save_processed_output.has_value() &&
             *convert.save_processed_output,
         "DecodeConvertRequest save_processed_output mismatch.", failures);
  Expect(convert.validate_logic.has_value() && !*convert.validate_logic,
         "DecodeConvertRequest validate_logic mismatch.", failures);
  Expect(convert.validate_structure.has_value() && *convert.validate_structure,
         "DecodeConvertRequest validate_structure mismatch.", failures);

  const auto import = DecodeImportRequest(R"({"processed_path":"out/data"})");
  Expect(import.processed_path == "out/data",
         "DecodeImportRequest processed_path mismatch.", failures);

  const auto validate_structure =
      DecodeValidateStructureRequest(R"({"input_path":"test/data"})");
  Expect(validate_structure.input_path == "test/data",
         "DecodeValidateStructureRequest input_path mismatch.", failures);

  const auto validate_logic = DecodeValidateLogicRequest(
      R"({"input_path":"test/data","date_check_mode":"continuity"})");
  Expect(validate_logic.input_path == "test/data",
         "DecodeValidateLogicRequest input_path mismatch.", failures);
  Expect(validate_logic.date_check_mode.has_value() &&
             *validate_logic.date_check_mode == "continuity",
         "DecodeValidateLogicRequest date_check_mode mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeConvertRequest(R"({"input_path":1})"); },
      "field `input_path` must be a string.",
      "DecodeConvertRequest bad input_path type", failures);
  ExpectInvalidArgument(
      [] { (void)DecodeImportRequest(R"({"processed_path":1})"); },
      "field `processed_path` must be a string.",
      "DecodeImportRequest bad processed_path type", failures);
  ExpectInvalidArgument(
      [] {
        (void)DecodeValidateLogicRequest(
            R"({"input_path":"x","date_check_mode":1})");
      },
      "field `date_check_mode` must be a string.",
      "DecodeValidateLogicRequest bad date_check_mode type", failures);
}

void TestDecodeReportRequests(int& failures) {
  const auto single = DecodeReportRequest(
      R"({"type":"month","argument":"2026-01","format":"markdown"})");
  Expect(single.type == "month", "DecodeReportRequest type mismatch.", failures);
  Expect(single.argument == "2026-01",
         "DecodeReportRequest argument mismatch.", failures);
  Expect(single.format.has_value() && *single.format == "markdown",
         "DecodeReportRequest format mismatch.", failures);

  const auto batch =
      DecodeReportBatchRequest(R"({"days_list":[7,14,30],"format":"md"})");
  Expect(batch.days_list.size() == 3U,
         "DecodeReportBatchRequest list size mismatch.", failures);
  Expect(batch.days_list[1] == 14,
         "DecodeReportBatchRequest list value mismatch.", failures);
  Expect(batch.format.has_value() && *batch.format == "md",
         "DecodeReportBatchRequest format mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeReportBatchRequest(R"({"format":"md"})"); },
      "field `days_list` must be an integer array.",
      "DecodeReportBatchRequest missing days_list", failures);
}

void TestDecodeExportRequest(int& failures) {
  const auto request = DecodeExportRequest(
      R"({"type":"all-month","argument":"2026-01","format":"markdown","recent_days_list":[7,14]})");
  Expect(request.type == "all-month", "DecodeExportRequest type mismatch.",
         failures);
  Expect(request.argument.has_value() && *request.argument == "2026-01",
         "DecodeExportRequest argument mismatch.", failures);
  Expect(request.format.has_value() && *request.format == "markdown",
         "DecodeExportRequest format mismatch.", failures);
  Expect(request.recent_days_list.has_value() &&
             request.recent_days_list->size() == 2U,
         "DecodeExportRequest recent_days_list mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeExportRequest(R"({"argument":"x"})"); },
      "field `type` must be a string.", "DecodeExportRequest missing type",
      failures);
}

void TestDecodeTreeRequest(int& failures) {
  const auto request = DecodeTreeRequest(
      R"({"list_roots":true,"root_pattern":"study","max_depth":3})");
  Expect(request.list_roots.has_value() && *request.list_roots,
         "DecodeTreeRequest list_roots mismatch.", failures);
  Expect(request.root_pattern.has_value() && *request.root_pattern == "study",
         "DecodeTreeRequest root_pattern mismatch.", failures);
  Expect(request.max_depth.has_value() && *request.max_depth == 3,
         "DecodeTreeRequest max_depth mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeTreeRequest(R"({"max_depth":"3"})"); },
      "field `max_depth` must be an integer.",
      "DecodeTreeRequest max_depth type", failures);
}

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
      R"({"ok":true,"found":false,"error_message":"","roots":["study","sleep"],"nodes":[{"name":"study","children":[{"name":"math","children":[]}]}]})");
  Expect(response.ok, "DecodeTreeResponse ok mismatch.", failures);
  Expect(!response.found, "DecodeTreeResponse found mismatch.", failures);
  Expect(response.roots.size() == 2U,
         "DecodeTreeResponse roots size mismatch.", failures);
  Expect(response.nodes.size() == 1U,
         "DecodeTreeResponse nodes size mismatch.", failures);
  Expect(response.nodes[0].children.size() == 1U,
         "DecodeTreeResponse child size mismatch.", failures);

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
}

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
    const auto encoded = EncodeTreeRequest(request);
    const auto decoded = DecodeTreeRequest(encoded);
    Expect(decoded.list_roots == request.list_roots,
           "EncodeTreeRequest round-trip list_roots mismatch.", failures);
    Expect(decoded.root_pattern == request.root_pattern,
           "EncodeTreeRequest round-trip root_pattern mismatch.", failures);
    Expect(decoded.max_depth == request.max_depth,
           "EncodeTreeRequest round-trip max_depth mismatch.", failures);
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
  ProjectTreeNodePayload child{};
  child.name = "math";
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
  Expect(tree["nodes"][0].contains("children") &&
             tree["nodes"][0]["children"].is_array() &&
             tree["nodes"][0]["children"].size() == 1U,
         "EncodeTreeResponse child node mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestDecodeIngestRequest(failures);
  TestDecodeQueryRequest(failures);
  TestDecodeWorkflowRequests(failures);
  TestDecodeReportRequests(failures);
  TestDecodeExportRequest(failures);
  TestDecodeTreeRequest(failures);
  TestDecodeRuntimeCheckResponse(failures);
  TestDecodeResolveCliContextResponse(failures);
  TestDecodeTreeResponse(failures);
  TestEncodeRequestRoundTrip(failures);
  TestEncodeResponses(failures);
  TestEncodeTreeResponse(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_runtime_codec_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_runtime_codec_tests failures: "
            << failures << '\n';
  return 1;
}
