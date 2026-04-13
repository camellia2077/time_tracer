#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunReportingChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                        const fs::path& output_root) {
  constexpr std::size_t kSha256HexLength = 64U;

  const json kReportResponse =
      ParseResponse(api.runtime_report(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2025-01-03"},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "baseline runtime report");
  Require(kReportResponse.value("ok", false),
          "baseline runtime report should return ok=true");
  const std::string kReportHash =
      kReportResponse.value("report_hash_sha256", std::string{});
  Require(kReportHash.size() == kSha256HexLength,
          "baseline runtime report should include 64-char report hash");
  Require(
      kReportHash.find_first_not_of("0123456789abcdef") == std::string::npos,
      "baseline runtime report hash should be lower-hex");

  const json kReportResponseAgain =
      ParseResponse(api.runtime_report(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2025-01-03"},
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
  Require(kReportBatchHash.size() == kSha256HexLength,
          "baseline runtime report batch should include 64-char report hash");
  Require(kReportBatchHash != kReportHash ||
              kReportBatchContent !=
                  kReportResponse.value("content", std::string{}),
          "report and report-batch should not both match in hash+content for "
          "different requests");

  const json kTargetsResponse = ParseResponse(
      api.runtime_report(
          runtime,
          json{{"operation_kind", "targets"}, {"display_mode", "month"}}
              .dump()
              .c_str()),
      "baseline runtime report targets");
  Require(kTargetsResponse.value("ok", false),
          "baseline runtime report targets should return ok=true");
  Require(kTargetsResponse.value("type", std::string{}) == "month",
          "baseline runtime report targets should echo requested type");
  Require(kTargetsResponse.contains("items") && kTargetsResponse["items"].is_array(),
          "baseline runtime report targets should include array field `items`");
  Require(!kTargetsResponse["items"].empty(),
          "baseline runtime report targets should list at least one month");

  const json kEmptyRangeResponse = ParseResponse(
      api.runtime_report(
          runtime,
          json{{"operation_kind", "query"},
               {"display_mode", "range"},
               {"selection_kind", "date_range"},
               {"start_date", "2024-12-01"},
               {"end_date", "2024-12-31"},
               {"format", "markdown"}}
              .dump()
              .c_str()),
      "empty range runtime report");
  Require(kEmptyRangeResponse.value("ok", false),
          "empty range runtime report should return ok=true");
  Require(!kEmptyRangeResponse.contains("error_code") ||
              kEmptyRangeResponse.value("error_code", std::string{}).empty(),
          "empty range runtime report should not expose reporting.target.not_found");
  Require(kEmptyRangeResponse.value("has_records", true) == false,
          "empty range runtime report should expose has_records=false");
  Require(kEmptyRangeResponse.value("matched_day_count", -1) == 0,
          "empty range runtime report should expose matched_day_count=0");
  Require(kEmptyRangeResponse.value("matched_record_count", -1) == 0,
          "empty range runtime report should expose matched_record_count=0");
  Require(kEmptyRangeResponse.value("start_date", std::string{}) ==
              "2024-12-01",
          "empty range runtime report should expose start_date");
  Require(kEmptyRangeResponse.value("end_date", std::string{}) ==
              "2024-12-31",
          "empty range runtime report should expose end_date");
  Require(kEmptyRangeResponse.value("requested_days", 0) == 31,
          "empty range runtime report should expose requested_days=31");

  const json kMissingReportResponse =
      ParseResponse(api.runtime_report(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "day"},
                             {"selection_kind", "single_day"},
                             {"date", "2024-12-31"},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "missing runtime report target");
  Require(!kMissingReportResponse.value("ok", true),
          "missing runtime report target should return ok=false");
  Require(kMissingReportResponse.value("error_code", std::string{}) ==
              "reporting.target.not_found",
          "missing runtime report target should expose reporting.target.not_found");
  Require(kMissingReportResponse.value("error_category", std::string{}) ==
              "reporting",
          "missing runtime report target should expose reporting category");

  const json kMissingExportResponse = ParseResponse(
      api.runtime_report(
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
              "reporting.target.not_found",
          "missing runtime export target should expose reporting.target.not_found");
  Require(kMissingExportResponse.value("error_category", std::string{}) ==
              "reporting",
          "missing runtime export target should expose reporting category");

  RequireOk(api.runtime_report(
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

  RequireOk(api.runtime_report(
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

  const json kInvalidRecentReportResponse =
      ParseResponse(api.runtime_report(
                        runtime,
                        json{{"operation_kind", "query"},
                             {"display_mode", "recent"},
                             {"selection_kind", "recent_days"},
                             {"days", 0},
                             {"format", "markdown"}}
                            .dump()
                            .c_str()),
                    "invalid recent runtime report");
  Require(!kInvalidRecentReportResponse.value("ok", true),
          "invalid recent runtime report should return ok=false");
  Require(kInvalidRecentReportResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "invalid recent runtime report should use generic runtime error code");
  Require(kInvalidRecentReportResponse.value("error_message", std::string{}).find(
              "Recent argument must be a positive integer.") != std::string::npos,
          "invalid recent runtime report should explain positive-integer requirement");

  const json kInvalidTargetsResponse = ParseResponse(
      api.runtime_report(
          runtime,
          json{{"operation_kind", "targets"}, {"display_mode", "quarter"}}
              .dump()
              .c_str()),
      "invalid runtime report targets");
  Require(!kInvalidTargetsResponse.value("ok", true),
          "invalid runtime report targets should return ok=false");
  Require(kInvalidTargetsResponse.value("error_code", std::string{}) ==
              "runtime.generic_error",
          "invalid runtime report targets should use generic runtime error code");
  Require(kInvalidTargetsResponse.value("error_message", std::string{}).find(
              "field `type` must be one of") != std::string::npos,
          "invalid runtime report targets should explain supported target types");
}

}  // namespace tracer_core_c_api_stability_internal
