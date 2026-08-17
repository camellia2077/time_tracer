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

  const json kActivityHierarchyLeafMappingsResponse = ParseResponse(
      api.runtime_query(
          runtime, json{{"action", "activity_alias_mappings"}}.dump().c_str()),
      "baseline query activity alias mappings");
  Require(kActivityHierarchyLeafMappingsResponse.value("ok", false),
          "baseline query activity alias mappings should return ok=true");
  const json kActivityHierarchyLeafMappingsContent = json::parse(
      kActivityHierarchyLeafMappingsResponse.value("content", "{}"));
  Require(kActivityHierarchyLeafMappingsContent.contains("entries") &&
              kActivityHierarchyLeafMappingsContent["entries"].is_array(),
          "baseline query activity alias mappings content should include "
          "entries array");

  const json kMappingAliasKeysResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "mapping_alias_keys"}}.dump().c_str()),
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

  const json kAuthorableTokensResponse = ParseResponse(
      api.runtime_query(
          runtime, json{{"action", "authorable_event_tokens"}}.dump().c_str()),
      "baseline query authorable event tokens");
  Require(kAuthorableTokensResponse.value("ok", false),
          "baseline query authorable event tokens should return ok=true");
  const json kAuthorableTokensContent =
      json::parse(kAuthorableTokensResponse.value("content", "{}"));
  Require(kAuthorableTokensContent.contains("names") &&
              kAuthorableTokensContent["names"].is_array(),
          "baseline query authorable event tokens content should include names "
          "array");
  bool contains_canonical_authorable = false;
  for (const auto& item : kAuthorableTokensContent["names"]) {
    if (item.is_string() &&
        item.get<std::string>() == "recreation_game_clash-royale") {
      contains_canonical_authorable = true;
      break;
    }
  }
  Require(contains_canonical_authorable,
          "baseline query authorable event tokens should include canonical "
          "activity names.");

  const json kInsightsChartResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "insights_chart"}, {"lookback_days", 7}}
                            .dump()
                            .c_str()),
      "baseline query insights_chart");
  Require(kInsightsChartResponse.value("ok", false),
          "baseline query insights_chart should return ok=true");
  const json kInsightsChartContent =
      json::parse(kInsightsChartResponse.value("content", "{}"));
  Require(kInsightsChartContent.contains("roots") &&
              kInsightsChartContent["roots"].is_array(),
          "baseline query insights_chart content should include roots array");
  Require(kInsightsChartContent.contains("series") &&
              kInsightsChartContent["series"].is_array(),
          "baseline query insights_chart content should include series array");

  const json kInsightsChartRangeResponse = ParseResponse(
      api.runtime_query(runtime, json{{"action", "insights_chart"},
                                      {"from_date", "2026-01-01"},
                                      {"to_date", "2026-01-07"}}
                                     .dump()
                                     .c_str()),
      "baseline query insights_chart range");
  Require(kInsightsChartRangeResponse.value("ok", false),
          "baseline query insights_chart range should return ok=true");
  const json kInsightsChartRangeContent =
      json::parse(kInsightsChartRangeResponse.value("content", "{}"));
  Require(kInsightsChartRangeContent.contains("series") &&
              kInsightsChartRangeContent["series"].is_array(),
          "baseline query insights_chart range content should include series "
          "array");

  const json kInsightsCompositionResponse = ParseResponse(
      api.runtime_query(runtime, json{{"action", "insights_composition"},
                                      {"lookback_days", 7}}
                                     .dump()
                                     .c_str()),
      "baseline query insights_composition");
  Require(kInsightsCompositionResponse.value("ok", false),
          "baseline query insights_composition should return ok=true");
  const json kInsightsCompositionContent =
      json::parse(kInsightsCompositionResponse.value("content", "{}"));
  Require(
      kInsightsCompositionContent.contains("active_root_count") &&
          kInsightsCompositionContent["active_root_count"].is_number_integer(),
      "baseline query insights_composition should include active_root_count");
  Require(
      kInsightsCompositionContent.contains("tree") &&
          kInsightsCompositionContent["tree"].is_array(),
      "baseline query insights_composition should include weighted tree array");

  const json kTreeResponse = ParseResponse(
      api.runtime_query(runtime, json{{"action", "tree"},
                                      {"output_mode", "semantic_json"},
                                      {"root", "study"},
                                      {"tree_max_depth", 1},
                                      {"tree_period", "recent"},
                                      {"tree_period_argument", "7"}}
                                     .dump()
                                     .c_str()),
      "baseline semantic tree query");
  Require(kTreeResponse.value("ok", false),
          "baseline semantic tree query should return ok=true");
  const json kTreeContent = json::parse(kTreeResponse.value("content", "{}"));
  Require(kTreeContent.contains("roots") && kTreeContent["roots"].is_array(),
          "baseline semantic tree query should include roots array");
  Require(!kTreeContent["roots"].empty(),
          "baseline semantic tree query should return at least one node");

  const json kFirstNode = kTreeContent["roots"].front();
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

  const json kInvalidActionResponse = ParseResponse(
      api.runtime_query(runtime,
                        json{{"action", "not_a_real_action"}}.dump().c_str()),
      "baseline query invalid action");
  Require(!kInvalidActionResponse.value("ok", true),
          "baseline query invalid action should return ok=false");
  Require(
      kInvalidActionResponse.value("error_code", std::string{}) ==
          "runtime.generic_error",
      "baseline query invalid action should use generic runtime error code");
  Require(
      kInvalidActionResponse.value("error_message", std::string{})
              .find("field `action` must be one of") != std::string::npos,
      "baseline query invalid action should explain supported query actions");

  const json kInvalidChartRangeResponse = ParseResponse(
      api.runtime_query(runtime, json{{"action", "insights_chart"},
                                      {"from_date", "2026-01-07"},
                                      {"to_date", "2026-01-01"}}
                                     .dump()
                                     .c_str()),
      "baseline query invalid insights_chart range");
  Require(!kInvalidChartRangeResponse.value("ok", true),
          "baseline query invalid insights_chart range should return ok=false");
  Require(kInvalidChartRangeResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "baseline query invalid insights_chart range should use generic "
          "runtime error code");
  Require(kInvalidChartRangeResponse.value("error_message", std::string{})
                  .find("insights-chart invalid range") != std::string::npos,
          "baseline query invalid insights_chart range should explain "
          "descending ranges");

  const json kInvalidCompositionRangeResponse = ParseResponse(
      api.runtime_query(runtime, json{{"action", "insights_composition"},
                                      {"from_date", "2026-01-07"},
                                      {"to_date", "2026-01-01"}}
                                     .dump()
                                     .c_str()),
      "baseline query invalid insights_composition range");
  Require(!kInvalidCompositionRangeResponse.value("ok", true),
          "baseline query invalid insights_composition range should return "
          "ok=false");
  Require(
      kInvalidCompositionRangeResponse.value("error_message", std::string{})
              .find("insights-composition invalid range") != std::string::npos,
      "baseline query invalid insights_composition range should explain "
      "descending ranges");
}

}  // namespace tracer_core_c_api_stability_internal
