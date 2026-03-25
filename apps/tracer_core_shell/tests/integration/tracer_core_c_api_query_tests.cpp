// tests/integration/tracer_core_c_api_query_tests.cpp
#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

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

