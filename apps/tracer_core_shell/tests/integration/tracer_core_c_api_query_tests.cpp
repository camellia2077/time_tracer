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

  const json kMappingAliasKeysResponse =
      ParseResponse(api.runtime_query(runtime,
                                      json{{"action", "mapping_alias_keys"}}
                                          .dump()
                                          .c_str()),
                    "baseline query mapping alias keys");
  Require(kMappingAliasKeysResponse.value("ok", false),
          "baseline query mapping alias keys should return ok=true");
  const json kMappingAliasKeysContent =
      json::parse(kMappingAliasKeysResponse.value("content", "{}"));
  Require(kMappingAliasKeysContent.contains("names") &&
              kMappingAliasKeysContent["names"].is_array(),
          "baseline query mapping alias keys content should include names "
          "array");

  const json kWakeKeywordsResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "wake_keywords"}}.dump().c_str()),
      "baseline query wake keywords");
  Require(kWakeKeywordsResponse.value("ok", false),
          "baseline query wake keywords should return ok=true");
  const json kWakeKeywordsContent =
      json::parse(kWakeKeywordsResponse.value("content", "{}"));
  Require(kWakeKeywordsContent.contains("names") &&
              kWakeKeywordsContent["names"].is_array(),
          "baseline query wake keywords content should include names array");

  const json kAuthorableTokensResponse =
      ParseResponse(api.runtime_query(runtime,
                                      json{{"action", "authorable_event_tokens"}}
                                          .dump()
                                          .c_str()),
                    "baseline query authorable event tokens");
  Require(kAuthorableTokensResponse.value("ok", false),
          "baseline query authorable event tokens should return ok=true");
  const json kAuthorableTokensContent =
      json::parse(kAuthorableTokensResponse.value("content", "{}"));
  Require(kAuthorableTokensContent.contains("names") &&
              kAuthorableTokensContent["names"].is_array(),
          "baseline query authorable event tokens content should include names "
          "array");

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

  const json kReportCompositionResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "report_composition"},
                             {"lookback_days", 7}}
                            .dump()
                            .c_str()),
      "baseline query report_composition");
  Require(kReportCompositionResponse.value("ok", false),
          "baseline query report_composition should return ok=true");
  const json kReportCompositionContent =
      json::parse(kReportCompositionResponse.value("content", "{}"));
  Require(kReportCompositionContent.contains("slices") &&
              kReportCompositionContent["slices"].is_array(),
          "baseline query report_composition content should include slices array");
  Require(kReportCompositionContent.contains("active_root_count") &&
              kReportCompositionContent["active_root_count"].is_number_integer(),
          "baseline query report_composition should include active_root_count");

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

  const json kInvalidActionResponse =
      ParseResponse(api.runtime_query(runtime,
                                      json{{"action", "not_a_real_action"}}
                                          .dump()
                                          .c_str()),
                    "baseline query invalid action");
  Require(!kInvalidActionResponse.value("ok", true),
          "baseline query invalid action should return ok=false");
  Require(kInvalidActionResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "baseline query invalid action should use generic runtime error code");
  Require(kInvalidActionResponse.value("error_message", std::string{}).find(
              "field `action` must be one of") != std::string::npos,
          "baseline query invalid action should explain supported query actions");

  const json kInvalidChartRangeResponse =
      ParseResponse(api.runtime_query(runtime, json{{"action", "report_chart"},
                                                    {"from_date", "2026-01-07"},
                                                    {"to_date", "2026-01-01"}}
                                                   .dump()
                                                   .c_str()),
                    "baseline query invalid report_chart range");
  Require(!kInvalidChartRangeResponse.value("ok", true),
          "baseline query invalid report_chart range should return ok=false");
  Require(kInvalidChartRangeResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "baseline query invalid report_chart range should use generic runtime error code");
  Require(kInvalidChartRangeResponse.value("error_message", std::string{}).find(
              "report-chart invalid range") != std::string::npos,
          "baseline query invalid report_chart range should explain descending ranges");

  const json kInvalidCompositionRangeResponse =
      ParseResponse(api.runtime_query(runtime,
                                      json{{"action", "report_composition"},
                                           {"from_date", "2026-01-07"},
                                           {"to_date", "2026-01-01"}}
                                          .dump()
                                          .c_str()),
                    "baseline query invalid report_composition range");
  Require(!kInvalidCompositionRangeResponse.value("ok", true),
          "baseline query invalid report_composition range should return ok=false");
  Require(kInvalidCompositionRangeResponse.value("error_message", std::string{})
              .find("report-composition invalid range") != std::string::npos,
          "baseline query invalid report_composition range should explain descending ranges");
}

}  // namespace tracer_core_c_api_stability_internal
