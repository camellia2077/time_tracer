// api/core_c/tracer_core_c_api_query_tests.cpp
#include "api/core_c/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunQueryChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime) {
  RequireOk(
      api.runtime_query(runtime, json{{"action", "years"}}.dump().c_str()),
      "baseline query years");

  const json kMappingNamesResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "mapping_names"}}.dump().c_str()),
      "baseline query mapping names");
  Require(kMappingNamesResponse.value("ok", false),
          "baseline query mapping names should return ok=true");
  const json kMappingNamesContent =
      json::parse(kMappingNamesResponse.value("content", "{}"));
  Require(kMappingNamesContent.contains("names") &&
              kMappingNamesContent["names"].is_array(),
          "baseline query mapping names content should include names array");

  const json kReportChartResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "report_chart"}, {"lookback_days", 7}}
                            .dump()
                            .c_str()),
      "baseline query report_chart");
  Require(kReportChartResponse.value("ok", false),
          "baseline query report_chart should return ok=true");
  const json kReportChartContent =
      json::parse(kReportChartResponse.value("content", "{}"));
  Require(kReportChartContent.contains("roots") &&
              kReportChartContent["roots"].is_array(),
          "baseline query report_chart content should include roots array");
  Require(kReportChartContent.contains("series") &&
              kReportChartContent["series"].is_array(),
          "baseline query report_chart content should include series array");

  const json kReportChartRangeResponse =
      ParseResponse(api.runtime_query(runtime, json{{"action", "report_chart"},
                                                    {"from_date", "2026-01-01"},
                                                    {"to_date", "2026-01-07"}}
                                                   .dump()
                                                   .c_str()),
                    "baseline query report_chart range");
  Require(kReportChartRangeResponse.value("ok", false),
          "baseline query report_chart range should return ok=true");
  const json kReportChartRangeContent =
      json::parse(kReportChartRangeResponse.value("content", "{}"));
  Require(
      kReportChartRangeContent.contains("series") &&
          kReportChartRangeContent["series"].is_array(),
      "baseline query report_chart range content should include series array");

  const json kReportResponse =
      ParseResponse(api.runtime_report(runtime, json{{"type", "day"},
                                                     {"argument", "2025-01-03"},
                                                     {"format", "markdown"}}
                                                    .dump()
                                                    .c_str()),
                    "baseline runtime report");
  Require(kReportResponse.value("ok", false),
          "baseline runtime report should return ok=true");
  const std::string kReportHash =
      kReportResponse.value("report_hash_sha256", std::string{});
  Require(kReportHash.size() == 64,
          "baseline runtime report should include 64-char report hash");
  Require(
      kReportHash.find_first_not_of("0123456789abcdef") == std::string::npos,
      "baseline runtime report hash should be lower-hex");

  const json kReportResponseAgain =
      ParseResponse(api.runtime_report(runtime, json{{"type", "day"},
                                                     {"argument", "2025-01-03"},
                                                     {"format", "markdown"}}
                                                    .dump()
                                                    .c_str()),
                    "baseline runtime report repeat");
  Require(kReportResponseAgain.value("ok", false),
          "baseline runtime report repeat should return ok=true");
  const std::string kReportHashAgain =
      kReportResponseAgain.value("report_hash_sha256", std::string{});
  Require(kReportHashAgain == kReportHash,
          "baseline runtime report hash should be stable for same request");
  Require(kReportResponseAgain.value("content", std::string{}) ==
              kReportResponse.value("content", std::string{}),
          "baseline runtime report content should be stable for same request");

  const json kReportBatchResponse = ParseResponse(
      api.runtime_report_batch(
          runtime,
          json{{"days_list", json::array({1, 3, 5})}, {"format", "markdown"}}
              .dump()
              .c_str()),
      "baseline runtime report batch");
  Require(kReportBatchResponse.value("ok", false),
          "baseline runtime report batch should return ok=true");
  const std::string kReportBatchContent =
      kReportBatchResponse.value("content", std::string{});
  const std::string kReportBatchHash =
      kReportBatchResponse.value("report_hash_sha256", std::string{});
  Require(kReportBatchHash.size() == 64,
          "baseline runtime report batch should include 64-char report hash");
  Require(kReportBatchHash != kReportHash ||
              kReportBatchContent !=
                  kReportResponse.value("content", std::string{}),
          "report and report-batch should not both match in hash+content for "
          "different requests");

  const json kTreeRootsResponse = ParseResponse(
      api.runtime_tree(runtime, json{{"list_roots", true}}.dump().c_str()),
      "baseline runtime tree roots");
  Require(kTreeRootsResponse.value("ok", false),
          "baseline runtime tree roots should return ok=true");
  Require(kTreeRootsResponse.contains("roots") &&
              kTreeRootsResponse["roots"].is_array(),
          "baseline runtime tree roots response should include roots array");

  const json kTreeResponse =
      ParseResponse(api.runtime_tree(runtime, json{{"root_pattern", "study"},
                                                   {"max_depth", 1},
                                                   {"period", "recent"},
                                                   {"period_argument", "7"}}
                                                  .dump()
                                                  .c_str()),
                    "baseline runtime tree query");
  Require(kTreeResponse.value("ok", false),
          "baseline runtime tree query should return ok=true");
  Require(kTreeResponse.value("found", false),
          "baseline runtime tree query should return found=true");
  Require(kTreeResponse.contains("nodes") && kTreeResponse["nodes"].is_array(),
          "baseline runtime tree query should include nodes array");
  Require(!kTreeResponse["nodes"].empty(),
          "baseline runtime tree query should return at least one node");

  const json kFirstNode = kTreeResponse["nodes"].front();
  Require(kFirstNode.contains("name") && kFirstNode["name"].is_string(),
          "baseline runtime tree node should include string name");
  Require(kFirstNode.contains("children") && kFirstNode["children"].is_array(),
          "baseline runtime tree node should include children array");
  Require(kFirstNode.contains("path") && kFirstNode["path"].is_string(),
          "baseline runtime tree node should include string path");
  if (kFirstNode.contains("duration_seconds")) {
    Require(kFirstNode["duration_seconds"].is_number_integer(),
            "baseline runtime tree node duration_seconds should be integer "
            "when present");
  }
}

}  // namespace tracer_core_c_api_stability_internal
