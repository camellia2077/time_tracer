// tests/transport_runtime_codec_decode_request_tests.cpp
#include "transport_runtime_codec_test_common.hpp"

namespace tracer_transport_runtime_codec_tests {
namespace {

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
      R"({"list_roots":true,"root_pattern":"study","max_depth":3,"period":"recent","period_argument":"7","root":"study"})");
  Expect(request.list_roots.has_value() && *request.list_roots,
         "DecodeTreeRequest list_roots mismatch.", failures);
  Expect(request.root_pattern.has_value() && *request.root_pattern == "study",
         "DecodeTreeRequest root_pattern mismatch.", failures);
  Expect(request.max_depth.has_value() && *request.max_depth == 3,
         "DecodeTreeRequest max_depth mismatch.", failures);
  Expect(request.period.has_value() && *request.period == "recent",
         "DecodeTreeRequest period mismatch.", failures);
  Expect(request.period_argument.has_value() &&
             *request.period_argument == "7",
         "DecodeTreeRequest period_argument mismatch.", failures);
  Expect(request.root.has_value() && *request.root == "study",
         "DecodeTreeRequest root mismatch.", failures);

  ExpectInvalidArgument(
      [] { (void)DecodeTreeRequest(R"({"max_depth":"3"})"); },
      "field `max_depth` must be an integer.",
      "DecodeTreeRequest max_depth type", failures);
  ExpectInvalidArgument(
      [] { (void)DecodeTreeRequest(R"({"period":1})"); },
      "field `period` must be a string.", "DecodeTreeRequest period type",
      failures);
}

}  // namespace

auto RunDecodeRequestTests(int& failures) -> void {
  TestDecodeIngestRequest(failures);
  TestDecodeQueryRequest(failures);
  TestDecodeWorkflowRequests(failures);
  TestDecodeReportRequests(failures);
  TestDecodeExportRequest(failures);
  TestDecodeTreeRequest(failures);
}

}  // namespace tracer_transport_runtime_codec_tests
