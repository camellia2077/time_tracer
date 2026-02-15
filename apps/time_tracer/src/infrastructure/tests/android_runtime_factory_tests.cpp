#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "infrastructure/bootstrap/android_runtime_factory.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"

namespace {

auto Contains(const std::string& text, const std::string& keyword) -> bool {
  return text.find(keyword) != std::string::npos;
}

auto RemoveTree(const std::filesystem::path& path) -> void {
  std::error_code error;
  std::filesystem::remove_all(path, error);
}

auto TestAndroidRuntimeCanRunDataQuery(int& failures) -> void {
  const std::filesystem::path kTestRoot =
      std::filesystem::temp_directory_path() /
      "time_tracer_android_runtime_factory_test";
  const std::filesystem::path kOutputRoot = kTestRoot / "output";
  const std::filesystem::path kDbPath = kOutputRoot / "db" / "android.sqlite3";
  const std::filesystem::path kConfigTomlPath =
      std::filesystem::path(__FILE__)
          .parent_path()
          .parent_path()
          .parent_path() /
      ".." / "config" / "converter" / "interval_processor_config.toml";

  RemoveTree(kTestRoot);

  try {
    infrastructure::bootstrap::AndroidRuntimeRequest request;
    request.output_root = kOutputRoot;
    request.db_path = kDbPath;
    request.converter_config_toml_path = kConfigTomlPath;

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid core API.\n";
      RemoveTree(kTestRoot);
      return;
    }

    time_tracer::core::dto::DataQueryRequest query_request;
    query_request.action = time_tracer::core::dto::DataQueryAction::kYears;

    const auto query_result = runtime.core_api->RunDataQuery(query_request);
    if (!query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery should succeed: "
                << query_result.error_message << '\n';
    }
    if (!Contains(query_result.content, "Total: 0")) {
      ++failures;
      std::cerr
          << "[FAIL] RunDataQuery years output should include 'Total: 0'.\n";
    }

    time_tracer::core::dto::DataQueryRequest stats_query_request;
    stats_query_request.action =
        time_tracer::core::dto::DataQueryAction::kDaysStats;
    const auto stats_query_result =
        runtime.core_api->RunDataQuery(stats_query_request);
    if (!stats_query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) should succeed: "
                << stats_query_result.error_message << '\n';
    } else if (!Contains(stats_query_result.content, "## Day Duration Stats")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                   "'## Day Duration Stats'.\n";
    } else if (!Contains(stats_query_result.content, "**Days**: 0")) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) output should include "
                   "'**Days**: 0'.\n";
    }

    const auto run_stats_period =
        [&](std::string period, std::optional<std::string> period_argument,
            std::optional<int> lookback_days) -> void {
      time_tracer::core::dto::DataQueryRequest request;
      request.action = time_tracer::core::dto::DataQueryAction::kDaysStats;
      request.tree_period = std::move(period);
      request.tree_period_argument = std::move(period_argument);
      request.lookback_days = lookback_days;

      const auto result = runtime.core_api->RunDataQuery(request);
      if (!result.ok) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') should succeed: " << result.error_message << '\n';
      } else if (!Contains(result.content, "## Day Duration Stats")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') output should include '## Day Duration Stats'.\n";
      } else if (!Contains(result.content, "**Days**: 0")) {
        ++failures;
        std::cerr << "[FAIL] RunDataQuery(days-stats, period='"
                  << *request.tree_period
                  << "') output should include '**Days**: 0'.\n";
      }
    };

    run_stats_period("day", std::string("2026-02-01"), std::nullopt);
    run_stats_period("week", std::string("2026-W05"), std::nullopt);
    run_stats_period("month", std::string("2026-02"), std::nullopt);
    run_stats_period("year", std::string("2026"), std::nullopt);
    run_stats_period("recent", std::string("7"), std::nullopt);
    run_stats_period("recent", std::nullopt, 7);
    run_stats_period("range", std::string("2026-02-01|2026-02-15"),
                     std::nullopt);

    time_tracer::core::dto::DataQueryRequest invalid_stats_period_request;
    invalid_stats_period_request.action =
        time_tracer::core::dto::DataQueryAction::kDaysStats;
    invalid_stats_period_request.tree_period = "invalid-period";
    invalid_stats_period_request.tree_period_argument = "1";
    const auto invalid_stats_period_result =
        runtime.core_api->RunDataQuery(invalid_stats_period_request);
    if (invalid_stats_period_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(days-stats) should reject invalid "
                   "period value.\n";
    }

    time_tracer::core::dto::DataQueryRequest tree_query_request;
    tree_query_request.action = time_tracer::core::dto::DataQueryAction::kTree;
    tree_query_request.tree_period = "recent";
    tree_query_request.tree_period_argument = "7";
    tree_query_request.tree_max_depth = 1;
    const auto tree_query_result =
        runtime.core_api->RunDataQuery(tree_query_request);
    if (!tree_query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(tree) should succeed: "
                << tree_query_result.error_message << '\n';
    } else if (!Contains(tree_query_result.content, "Total: 0")) {
      ++failures;
      std::cerr
          << "[FAIL] RunDataQuery(tree) output should include 'Total: 0'.\n";
    }

    const auto report_result = runtime.core_api->RunReportQuery(
        {.type = time_tracer::core::dto::ReportQueryType::kRecent,
         .argument = "1",
         .format = ReportFormat::kMarkdown});
    if (!report_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunReportQuery(markdown) should succeed: "
                << report_result.error_message << '\n';
    }
    if (report_result.content.empty()) {
      ++failures;
      std::cerr << "[FAIL] RunReportQuery(markdown) should return non-empty "
                   "content.\n";
    }

    const auto day_report_result = runtime.core_api->RunReportQuery(
        {.type = time_tracer::core::dto::ReportQueryType::kDay,
         .argument = "2026-02-01",
         .format = ReportFormat::kMarkdown});
    if (!day_report_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunReportQuery(day markdown) should succeed: "
                << day_report_result.error_message << '\n';
    } else {
      if (!Contains(day_report_result.content, "- **Date**: ")) {
        ++failures;
        std::cerr << "[FAIL] Android day markdown report should include "
                     "'Date' label.\n";
      }
      if (!Contains(day_report_result.content, "- **Total Time Recorded**: ")) {
        ++failures;
        std::cerr << "[FAIL] Android day markdown report should include "
                     "'Total Time Recorded' label.\n";
      }
    }

    const auto structured_result = runtime.core_api->RunStructuredReportQuery(
        {.type = time_tracer::core::dto::ReportQueryType::kRecent,
         .argument = "1"});
    if (!structured_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunStructuredReportQuery should succeed: "
                << structured_result.error_message << '\n';
    }
    if (structured_result.kind !=
        time_tracer::core::dto::StructuredReportKind::kRecent) {
      ++failures;
      std::cerr << "[FAIL] RunStructuredReportQuery should return kRecent.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Android runtime test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Android runtime test threw non-standard exception.\n";
  }

  RemoveTree(kTestRoot);
}

auto TestAndroidRuntimeRejectsInvalidConverterConfig(int& failures) -> void {
  const std::filesystem::path kTestRoot =
      std::filesystem::temp_directory_path() /
      "time_tracer_android_runtime_factory_invalid_config_test";
  const std::filesystem::path kOutputRoot = kTestRoot / "output";
  const std::filesystem::path kDbPath = kOutputRoot / "db" / "android.sqlite3";
  const std::filesystem::path kInvalidConfigPath = kTestRoot / "invalid.toml";

  RemoveTree(kTestRoot);
  std::filesystem::create_directories(kTestRoot);

  {
    std::ofstream file(kInvalidConfigPath);
    file << "remark_prefix = \"r \"\n";
  }

  bool threw = false;
  try {
    infrastructure::bootstrap::AndroidRuntimeRequest request;
    request.output_root = kOutputRoot;
    request.db_path = kDbPath;
    request.converter_config_toml_path = kInvalidConfigPath;

    static_cast<void>(infrastructure::bootstrap::BuildAndroidRuntime(request));
  } catch (const std::exception&) {
    threw = true;
  }

  if (!threw) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should fail for invalid converter "
                 "config TOML.\n";
  }

  RemoveTree(kTestRoot);
}

auto TestReportConfigLoaderRejectsInvalidDailyMarkdown(int& failures) -> void {
  const std::filesystem::path kTestRoot =
      std::filesystem::temp_directory_path() /
      "time_tracer_report_config_loader_invalid_test";
  const std::filesystem::path kInvalidReportPath =
      kTestRoot / "day_invalid.toml";

  RemoveTree(kTestRoot);
  std::filesystem::create_directories(kTestRoot);

  {
    std::ofstream file(kInvalidReportPath);
    file << "title_prefix = \"Daily Report for\"\n";
  }

  bool threw = false;
  std::string message;
  try {
    static_cast<void>(
        ReportConfigLoader::LoadDailyMdConfig(kInvalidReportPath));
  } catch (const std::exception& exception) {
    threw = true;
    message = exception.what();
  }

  if (!threw) {
    ++failures;
    std::cerr
        << "[FAIL] LoadDailyMdConfig should fail for invalid report config.\n";
  } else if (!Contains(message, "Invalid report config [") ||
             !Contains(message, "date_label")) {
    ++failures;
    std::cerr
        << "[FAIL] Invalid report config error should include context and "
           "missing key, actual: "
        << message << '\n';
  }

  RemoveTree(kTestRoot);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestAndroidRuntimeCanRunDataQuery(failures);
  TestAndroidRuntimeRejectsInvalidConverterConfig(failures);
  TestReportConfigLoaderRejectsInvalidDailyMarkdown(failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_android_runtime_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_android_runtime_tests failures: "
            << failures << '\n';
  return 1;
}
