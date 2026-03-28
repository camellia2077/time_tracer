#include "tests/integration/tracer_core_c_api_stability_internal.hpp"

namespace tracer_core_c_api_stability_internal {

void RunReportingChecks(const CoreApiFns& api, TtCoreRuntimeHandle* runtime,
                        const fs::path& output_root) {
  constexpr std::size_t kSha256HexLength = 64U;

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
  Require(kReportHash.size() == kSha256HexLength,
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
  Require(kReportBatchHash.size() == kSha256HexLength,
          "baseline runtime report batch should include 64-char report hash");
  Require(kReportBatchHash != kReportHash ||
              kReportBatchContent !=
                  kReportResponse.value("content", std::string{}),
          "report and report-batch should not both match in hash+content for "
          "different requests");

  const json kTargetsResponse = ParseResponse(
      api.runtime_report_targets(runtime, json{{"type", "month"}}.dump().c_str()),
      "baseline runtime report targets");
  Require(kTargetsResponse.value("ok", false),
          "baseline runtime report targets should return ok=true");
  Require(kTargetsResponse.value("type", std::string{}) == "month",
          "baseline runtime report targets should echo requested type");
  Require(kTargetsResponse.contains("items") && kTargetsResponse["items"].is_array(),
          "baseline runtime report targets should include array field `items`");
  Require(!kTargetsResponse["items"].empty(),
          "baseline runtime report targets should list at least one month");

  RequireOk(api.runtime_export(
                runtime,
                json{{"type", "month"},
                     {"argument", "2025-01"},
                     {"format", "md"}}
                    .dump()
                    .c_str()),
            "baseline runtime export single month");
  Require(fs::exists(output_root / "markdown" / "month" / "2025-01.md"),
          "baseline runtime export single month should write dashed legacy path");

  RequireOk(api.runtime_export(
                runtime,
                json{{"type", "all-month"}, {"format", "md"}}.dump().c_str()),
            "baseline runtime export all month");
  Require(fs::exists(output_root / "markdown" / "month" / "202501.md"),
          "baseline runtime export all month should preserve compact legacy layout");
}

}  // namespace tracer_core_c_api_stability_internal
