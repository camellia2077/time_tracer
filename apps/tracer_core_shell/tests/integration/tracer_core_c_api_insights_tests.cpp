#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunInsightsChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                        const fs::path& output_root) {
  constexpr std::size_t kSha256HexLength = 64U;

  const json kInsightsResponse =
      ParseResponse(api.runtime_insights(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2025-01-03"},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "baseline runtime insights");
  Require(kInsightsResponse.value("ok", false),
          "baseline runtime insights should return ok=true");
  const std::string kInsightsHash =
      kInsightsResponse.value("insights_hash_sha256", std::string{});
  Require(kInsightsHash.size() == kSha256HexLength,
          "baseline runtime insights should include 64-char insights hash");
  Require(
      kInsightsHash.find_first_not_of("0123456789abcdef") == std::string::npos,
      "baseline runtime insights hash should be lower-hex");

  const json kInsightsResponseAgain =
      ParseResponse(api.runtime_insights(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2025-01-03"},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "baseline runtime insights repeat");
  Require(kInsightsResponseAgain.value("ok", false),
          "baseline runtime insights repeat should return ok=true");
  const std::string kInsightsHashAgain =
      kInsightsResponseAgain.value("insights_hash_sha256", std::string{});
  Require(kInsightsHashAgain == kInsightsHash,
          "baseline runtime insights hash should be stable for same request");
  Require(kInsightsResponseAgain.value("content", std::string{}) ==
              kInsightsResponse.value("content", std::string{}),
          "baseline runtime insights content should be stable for same request");

  const json kStructuredInsightsResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "structured_query"},
               {"display_mode", "day"},
               {"selection_kind", "single_day"},
               {"date", "2025-01-03"}}
              .dump()
              .c_str()),
      "structured runtime insights");
  Require(kStructuredInsightsResponse.value("ok", false),
          "structured runtime insights should return ok=true");
  Require(kStructuredInsightsResponse.value("insights_kind", std::string{}) ==
              "day",
          "structured runtime insights should identify day insights kind");
  const auto& kStructuredMetadata =
      kStructuredInsightsResponse.at("insights").at("metadata");
  Require(kStructuredMetadata.contains("statuses") &&
              kStructuredMetadata.at("statuses").is_array(),
          "structured day metadata should expose statuses array");
  Require(kStructuredMetadata.at("statuses").size() == 2U,
          "structured day metadata should expose configured statuses");
  Require(!kStructuredMetadata.contains("status") &&
              !kStructuredMetadata.contains("exercise"),
          "structured day metadata should not expose fixed status fields");

  const json kStructuredRangeInsightsResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "structured_query"},
               {"display_mode", "range"},
               {"selection_kind", "date_range"},
               {"start_date", "2025-01-03"},
               {"end_date", "2025-01-03"}}
              .dump()
              .c_str()),
      "structured range runtime insights");
  Require(kStructuredRangeInsightsResponse.value("ok", false),
          "structured range runtime insights should return ok=true");
  const auto& kStructuredRangeStatuses =
      kStructuredRangeInsightsResponse.at("insights").at("statuses");
  Require(kStructuredRangeStatuses.is_array() &&
              kStructuredRangeStatuses.size() == 2U,
          "structured range insights should expose configured statuses");
  Require(kStructuredRangeStatuses.at(0).contains("occurrence_count") &&
              kStructuredRangeStatuses.at(0).contains("total_duration"),
          "structured range statuses should expose occurrence and duration totals");

  const json kRangeInsightsResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "query"},
               {"display_mode", "range"},
               {"selection_kind", "date_range"},
               {"start_date", "2025-01-03"},
               {"end_date", "2025-01-03"},
               {"format", "markdown"}}
              .dump()
              .c_str()),
      "range runtime insights with configured statuses");
  Require(kRangeInsightsResponse.value("ok", false),
          "range runtime insights should return ok=true");
  Require(kRangeInsightsResponse.value("content", std::string{}).find(
              kStructuredRangeStatuses.at(0).at("label").get<std::string>()) !=
              std::string::npos,
          "range Markdown insights should render configured status labels");

  const json kInsightsBatchResponse = ParseResponse(
      api.runtime_insights_batch(
          runtime,
          json{{"days_list", json::array({1, 3, 5})}, {"format", "markdown"}}
              .dump()
              .c_str()),
      "baseline runtime insights batch");
  Require(kInsightsBatchResponse.value("ok", false),
          "baseline runtime insights batch should return ok=true");
  const std::string kInsightsBatchContent =
      kInsightsBatchResponse.value("content", std::string{});
  const std::string kInsightsBatchHash =
      kInsightsBatchResponse.value("insights_hash_sha256", std::string{});
  Require(kInsightsBatchHash.size() == kSha256HexLength,
          "baseline runtime insights batch should include 64-char insights hash");
  Require(kInsightsBatchHash != kInsightsHash ||
              kInsightsBatchContent !=
                  kInsightsResponse.value("content", std::string{}),
          "insights and insights-batch should not both match in hash+content for "
          "different requests");

  const json kTargetsResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "targets"}, {"display_mode", "month"}}
              .dump()
              .c_str()),
      "baseline runtime insights targets");
  Require(kTargetsResponse.value("ok", false),
          "baseline runtime insights targets should return ok=true");
  Require(kTargetsResponse.value("type", std::string{}) == "month",
          "baseline runtime insights targets should echo requested type");
  Require(kTargetsResponse.contains("items") && kTargetsResponse["items"].is_array(),
          "baseline runtime insights targets should include array field `items`");
  Require(!kTargetsResponse["items"].empty(),
          "baseline runtime insights targets should list at least one month");

  const json kEmptyRangeResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "query"},
               {"display_mode", "range"},
               {"selection_kind", "date_range"},
               {"start_date", "2024-12-01"},
               {"end_date", "2024-12-31"},
               {"format", "markdown"}}
              .dump()
              .c_str()),
      "empty range runtime insights");
  Require(kEmptyRangeResponse.value("ok", false),
          "empty range runtime insights should return ok=true");
  Require(!kEmptyRangeResponse.contains("error_code") ||
              kEmptyRangeResponse.value("error_code", std::string{}).empty(),
          "empty range runtime insights should not expose insights.target.not_found");
  Require(kEmptyRangeResponse.value("has_records", true) == false,
          "empty range runtime insights should expose has_records=false");
  Require(kEmptyRangeResponse.value("matched_day_count", -1) == 0,
          "empty range runtime insights should expose matched_day_count=0");
  Require(kEmptyRangeResponse.value("matched_record_count", -1) == 0,
          "empty range runtime insights should expose matched_record_count=0");
  Require(kEmptyRangeResponse.value("start_date", std::string{}) ==
              "2024-12-01",
          "empty range runtime insights should expose start_date");
  Require(kEmptyRangeResponse.value("end_date", std::string{}) ==
              "2024-12-31",
          "empty range runtime insights should expose end_date");
  Require(kEmptyRangeResponse.value("requested_days", 0) == 31,
          "empty range runtime insights should expose requested_days=31");

  const json kMissingInsightsResponse =
      ParseResponse(api.runtime_insights(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2024-12-31"},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "missing runtime insights target");
  Require(!kMissingInsightsResponse.value("ok", true),
          "missing runtime insights target should return ok=false");
  Require(kMissingInsightsResponse.value("error_code", std::string{}) ==
              "insights.target.not_found",
          "missing runtime insights target should expose insights.target.not_found");
  Require(kMissingInsightsResponse.value("error_category", std::string{}) ==
              "insights",
          "missing runtime insights target should expose insights category");

  const json kMissingExportResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "export"},
               {"display_mode", "month"},
               {"export_scope", "single"},
               {"selection_kind", "date_range"},
               {"start_date", "2024-12-01"},
               {"end_date", "2024-12-31"},
               {"format", "md"}}
              .dump()
              .c_str()),
      "missing runtime export target");
  Require(!kMissingExportResponse.value("ok", true),
          "missing runtime export target should return ok=false");
  Require(kMissingExportResponse.value("error_code", std::string{}) ==
              "insights.target.not_found",
          "missing runtime export target should expose insights.target.not_found");
  Require(kMissingExportResponse.value("error_category", std::string{}) ==
              "insights",
          "missing runtime export target should expose insights category");

  RequireOk(api.runtime_insights(
                runtime,
                json{{"operation_kind", "export"},
                     {"display_mode", "month"},
                     {"export_scope", "single"},
                     {"selection_kind", "date_range"},
                     {"start_date", "2025-01-01"},
                     {"end_date", "2025-01-31"},
                     {"format", "md"}}
                    .dump()
                    .c_str()),
            "baseline runtime export single month");
  Require(fs::exists(output_root / "markdown" / "month" / "2025-01.md"),
          "baseline runtime export single month should write dashed legacy path");

  RequireOk(api.runtime_insights(
                runtime,
                json{{"operation_kind", "export"},
                     {"display_mode", "month"},
                     {"export_scope", "all_matching"},
                     {"format", "md"}}
                    .dump()
                    .c_str()),
            "baseline runtime export all month");
  Require(fs::exists(output_root / "markdown" / "month" / "2025-01.md"),
          "baseline runtime export all month should preserve month output layout");

  const json kInvalidRecentInsightsResponse =
      ParseResponse(api.runtime_insights(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "recent"},
                             {"selection_kind", "recent_days"},
                             {"days", 0},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "invalid recent runtime insights");
  Require(!kInvalidRecentInsightsResponse.value("ok", true),
          "invalid recent runtime insights should return ok=false");
  Require(kInvalidRecentInsightsResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "invalid recent runtime insights should use generic runtime error code");
  Require(kInvalidRecentInsightsResponse.value("error_message", std::string{}).find(
              "Recent argument must be a positive integer.") != std::string::npos,
          "invalid recent runtime insights should explain positive-integer requirement");

  const json kInvalidTargetsResponse = ParseResponse(
      api.runtime_insights(
          runtime,
          json{{"operation_kind", "targets"}, {"display_mode", "quarter"}}
              .dump()
              .c_str()),
      "invalid runtime insights targets");
  Require(!kInvalidTargetsResponse.value("ok", true),
          "invalid runtime insights targets should return ok=false");
  Require(kInvalidTargetsResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "invalid runtime insights targets should use generic runtime error code");
  Require(kInvalidTargetsResponse.value("error_message", std::string{}).find(
              "field `type` must be one of") != std::string::npos,
          "invalid runtime insights targets should explain supported target types");
}

}  // namespace tracer_core_c_api_stability_internal
