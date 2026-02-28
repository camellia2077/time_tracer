// infrastructure/tests/data_query/data_query_refactor_tree_tests.cpp
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/renderers/data_query_renderer.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"
#include "infrastructure/tests/data_query/data_query_refactor_test_internal.hpp"

namespace android_runtime_tests::data_query_refactor_internal {
namespace {

using nlohmann::json;

auto TestRendererGateway(int& failures) -> void {
  const std::vector<std::string> kYears = {"2024", "2025"};

  const std::string kText =
      tracer_core::infrastructure::query::data::renderers::RenderListOutput(
          "years", kYears, false);
  Expect(Contains(kText, "Total: 2"),
         "text renderer should preserve total footer.", failures);

  const std::string kSemantic =
      tracer_core::infrastructure::query::data::renderers::RenderListOutput(
          "years", kYears, true);
  const auto kSemanticJson = json::parse(kSemantic);
  Expect(kSemanticJson.value("schema_version", 0) == 1,
         "semantic renderer should emit schema_version.", failures);
  Expect(kSemanticJson.value("action", std::string{}) == "years",
         "semantic renderer should emit action field.", failures);
  Expect(kSemanticJson.value("output_mode", std::string{}) == "semantic_json",
         "semantic renderer should emit output_mode.", failures);

  const std::string kWrappedRaw = tracer_core::infrastructure::query::data::
      renderers::RenderJsonObjectOutput("report_chart", "not-json", true);
  const auto kWrappedRawJson = json::parse(kWrappedRaw);
  Expect(kWrappedRawJson.contains("raw_content"),
         "semantic json wrapper should preserve raw payload on parse failure.",
         failures);
}

auto BuildNode(const std::string& name, const std::string& path,
               long long duration, std::vector<ProjectTreeNode> children = {})
    -> ProjectTreeNode {
  ProjectTreeNode node{};
  node.name = name;
  node.path = path;
  node.duration_seconds = duration;
  node.children = std::move(children);
  return node;
}

auto TestTreeRendererWithStructuredNodes(int& failures) -> void {
  constexpr int kMissingMaxDepthSentinel = -99;
  constexpr long long kMathDurationSeconds = 3600LL;
  constexpr long long kAlgebraDurationSeconds = 1200LL;
  constexpr long long kPhysicsDurationSeconds = 1800LL;
  constexpr long long kStudyDurationSeconds = 7200LL;
  std::vector<ProjectTreeNode> nodes = {
      BuildNode(
          "study", "study", kStudyDurationSeconds,
          {BuildNode("math", "study_math", kMathDurationSeconds,
                     {BuildNode("algebra", "study_math_algebra",
                                kAlgebraDurationSeconds)}),
           BuildNode("physics", "study_physics", kPhysicsDurationSeconds)}),
  };

  const std::string kText = tracer_core::infrastructure::query::data::
      renderers::RenderProjectTreeOutput(nodes, 1, false);
  Expect(Contains(kText, "study\n"), "tree text renderer should output root.",
         failures);
  Expect(Contains(kText, "math"), "tree text renderer should output children.",
         failures);
  Expect(!Contains(kText, "algebra"),
         "tree text renderer should respect max_depth trimming.", failures);

  const std::string kSemantic = tracer_core::infrastructure::query::data::
      renderers::RenderProjectTreeOutput(nodes, 1, true);
  const auto kPayload = json::parse(kSemantic);
  Expect(kPayload.value("action", std::string{}) == "tree",
         "semantic tree payload should include action=tree.", failures);
  Expect(kPayload.value("max_depth", kMissingMaxDepthSentinel) == 1,
         "semantic tree payload should include max_depth.", failures);
  Expect(kPayload.value("root_count", 0) == 1,
         "semantic tree payload should include root_count.", failures);
  Expect(kPayload["roots"][0].value("duration_seconds", -1LL) ==
             kStudyDurationSeconds,
         "semantic tree payload should preserve node duration.", failures);
  Expect(kPayload["roots"][0]["children"].size() == 2U,
         "semantic tree payload should include first-level children.",
         failures);
  Expect(kPayload["roots"][0]["children"][0]["children"].empty(),
         "semantic tree payload should respect max_depth trimming.", failures);
}

auto TestAdapterBoundaryGuardrails(int& failures) -> void {
  struct AdapterGuardRule {
    std::filesystem::path relative_path;
    std::vector<std::string> forbidden_tokens;
  };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();

  const std::vector<AdapterGuardRule> kRules = {
      {
          "apps/tracer_cli/windows/src/api/cli/impl/commands/query/"
          "data_query_parser.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_cli/windows/src/api/cli/impl/commands/query/"
          "query_command.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_cli/windows/src/bootstrap/cli_runtime_factory_proxy.cpp",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },
      {
          "apps/tracer_android/runtime/src/main/java/com/example/tracer/"
          "runtime/"
          "controller/RuntimeQueryDelegate.kt",
          {"ComputeDayDurationStats(", "BuildReportChartSeries(",
           "ResolveExplicitDateRange(", "ResolveRollingDateRange(",
           "variance_seconds", "stddev_seconds", "mad_seconds"},
      },

      {
          "apps/tracer_core/src/infrastructure/persistence/"
          "sqlite_data_query_service.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
      {
          "apps/tracer_core/src/infrastructure/persistence/"
          "sqlite_data_query_service_dispatch.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
      {
          "apps/tracer_core/src/infrastructure/persistence/"
          "sqlite_data_query_service_report_mapping.cpp",
          {"std::sqrt(", "nearest-rank",
           "variance_seconds =", "stddev_seconds ="},
      },
  };

  for (const auto& rule : kRules) {
    const std::filesystem::path kTarget = kRepoRoot / rule.relative_path;
    const auto kContent = ReadFileText(kTarget);
    Expect(
        kContent.has_value(),
        std::string("adapter guardrails should read file: ") + kTarget.string(),
        failures);
    if (!kContent.has_value()) {
      continue;
    }
    for (const auto& token : rule.forbidden_tokens) {
      if (Contains(*kContent, token)) {
        ++failures;
        std::cerr << "[FAIL] adapter guardrail hit forbidden token `" << token
                  << "` in " << kTarget.string() << '\n';
      }
    }
  }
}

}  // namespace

auto RunDataQueryRefactorTreeTests(int& failures) -> void {
  TestRendererGateway(failures);
  TestTreeRendererWithStructuredNodes(failures);
  TestAdapterBoundaryGuardrails(failures);
}

}  // namespace android_runtime_tests::data_query_refactor_internal

