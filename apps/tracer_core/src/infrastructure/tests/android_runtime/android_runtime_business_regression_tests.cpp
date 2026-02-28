// infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp
#include <sqlite3.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "infrastructure/reports/data/cache/project_name_cache.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto TestProjectRenameKeepsHistorySnapshot(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_project_rename_history_test");
  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kInputPath = kRepoRoot / "test" / "data";
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "apps" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);

    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid core API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = kInputPath.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result = runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunIngest should succeed: "
                << ingest_result.error_message << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &db) != SQLITE_OK ||
        db == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed for runtime database.\n";
      if (db != nullptr) {
        sqlite3_close(db);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const bool renamed = ExecuteSql(db,
                                    "UPDATE projects "
                                    "SET name = 'english_renamed' "
                                    "WHERE full_path = 'study_english';");
    sqlite3_close(db);

    if (!renamed) {
      ++failures;
      std::cerr << "[FAIL] Renaming project node in sqlite should succeed.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::DataQueryRequest tree_query_request;
    tree_query_request.action = tracer_core::core::dto::DataQueryAction::kTree;
    tree_query_request.tree_period = "range";
    tree_query_request.tree_period_argument = "2026-01-01|2026-01-10";
    const auto tree_query_result =
        runtime.core_api->RunDataQuery(tree_query_request);

    if (!tree_query_result.ok) {
      ++failures;
      std::cerr << "[FAIL] RunDataQuery(tree) should succeed after project "
                   "rename: "
                << tree_query_result.error_message << '\n';
    } else if (Contains(tree_query_result.content, "english_renamed")) {
      ++failures;
      std::cerr << "[FAIL] Tree query should not rewrite historical paths "
                   "after project rename.\n";
    } else if (!Contains(tree_query_result.content, "english")) {
      ++failures;
      std::cerr << "[FAIL] Tree query should still include historical "
                   "'english' node.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Rename/history snapshot test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Rename/history snapshot test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

auto QueryCount(sqlite3* database, const std::string& sql)
    -> std::optional<long long> {
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(database, sql.c_str(), -1, &statement, nullptr) !=
          SQLITE_OK ||
      statement == nullptr) {
    return std::nullopt;
  }

  std::optional<long long> result;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    result = sqlite3_column_int64(statement, 0);
  }
  sqlite3_finalize(statement);
  return result;
}

auto TestProjectNameCacheRefreshesAfterProjectTableChanges(int& failures)
    -> void {
  sqlite3* database = nullptr;
  if (sqlite3_open(":memory:", &database) != SQLITE_OK || database == nullptr) {
    ++failures;
    std::cerr << "[FAIL] sqlite3_open(:memory:) should succeed for "
                 "ProjectNameCache refresh test.\n";
    if (database != nullptr) {
      sqlite3_close(database);
    }
    return;
  }

  const bool created = ExecuteSql(database,
                                  "CREATE TABLE projects ("
                                  "id INTEGER PRIMARY KEY, "
                                  "name TEXT NOT NULL, "
                                  "parent_id INTEGER, "
                                  "full_path TEXT, "
                                  "depth INTEGER"
                                  ");");
  if (!created) {
    ++failures;
    std::cerr << "[FAIL] ProjectNameCache refresh test should create "
                 "projects table.\n";
    sqlite3_close(database);
    return;
  }

  const bool seeded_first = ExecuteSql(
      database,
      "INSERT INTO projects (id, name, parent_id, full_path, depth) VALUES "
      "(1, 'routine', NULL, 'routine', 1), "
      "(2, 'express', 1, 'routine_express', 2);");
  if (!seeded_first) {
    ++failures;
    std::cerr << "[FAIL] ProjectNameCache refresh test should seed first "
                 "project tree.\n";
    sqlite3_close(database);
    return;
  }

  ProjectNameCache cache;
  cache.EnsureLoaded(database);
  const auto first_path = cache.GetPathParts(2);
  if (first_path != std::vector<std::string>{"routine", "express"}) {
    ++failures;
    std::cerr << "[FAIL] ProjectNameCache refresh test should load initial "
                 "project path routine->express.\n";
    sqlite3_close(database);
    return;
  }

  const bool replaced_tree = ExecuteSql(
      database,
      "DELETE FROM projects; "
      "INSERT INTO projects (id, name, parent_id, full_path, depth) VALUES "
      "(1, 'recreation', NULL, 'recreation', 1), "
      "(2, 'game', 1, 'recreation_game', 2);");
  if (!replaced_tree) {
    ++failures;
    std::cerr << "[FAIL] ProjectNameCache refresh test should replace "
                 "project tree rows.\n";
    sqlite3_close(database);
    return;
  }

  cache.EnsureLoaded(database);
  const auto refreshed_path = cache.GetPathParts(2);
  if (refreshed_path != std::vector<std::string>{"recreation", "game"}) {
    ++failures;
    std::cerr << "[FAIL] ProjectNameCache should refresh mapping after "
                 "project table changes.\n";
  }

  sqlite3_close(database);
}

auto TestSingleTxtIngestFallsBackToSiblingPreviousMonthTxt(int& failures)
    -> void {
  const RuntimeTestPaths paths = BuildTempTestPaths(
      "time_tracer_single_txt_sibling_previous_month_fallback_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "apps" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.core_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid core API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path input_root = paths.test_root / "input";
    const std::filesystem::path previous_month_path =
        input_root / "history_2026_01.txt";
    const std::filesystem::path current_month_path =
        input_root / "current_2026_02.txt";
    const std::string previous_month_txt =
        "y2026\n"
        "m01\n"
        "0131\n"
        "2100w\n"
        "2200rest\n"
        "2330meal\n";
    const std::string current_month_txt =
        "y2026\n"
        "m02\n"
        "0201\n"
        "0700w\n"
        "0900rest\n"
        "1000meal\n";

    if (!WriteFileWithParents(previous_month_path, previous_month_txt) ||
        !WriteFileWithParents(current_month_path, current_month_txt)) {
      ++failures;
      std::cerr << "[FAIL] Failed to create sibling TXT fixtures for fallback "
                   "test.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = current_month_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    ingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;

    const auto ingest_result = runtime.core_api->RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Single TXT ingest with sibling fallback should "
                   "succeed: "
                << ingest_result.error_message << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    sqlite3* database = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed for fallback test.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const std::string sleep_link_sql =
        "SELECT COUNT(*) FROM time_records "
        "WHERE date='2026-02-01' "
        "AND project_path_snapshot='sleep_night' "
        "AND \"start\"='23:30' "
        "AND \"end\"='07:00';";
    const auto linked_count = QueryCount(database, sleep_link_sql);
    sqlite3_close(database);

    if (!linked_count.has_value()) {
      ++failures;
      std::cerr << "[FAIL] Failed to query sleep link count for fallback "
                   "verification.\n";
    } else if (*linked_count != 1) {
      ++failures;
      std::cerr << "[FAIL] Expected exactly one fallback-linked sleep record "
                   "(23:30->07:00), actual: "
                << *linked_count << '\n';
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Single TXT sibling fallback test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Single TXT sibling fallback test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunBusinessRegressionTests(int& failures) -> void {
  TestProjectRenameKeepsHistorySnapshot(failures);
  TestProjectNameCacheRefreshesAfterProjectTableChanges(failures);
  TestSingleTxtIngestFallsBackToSiblingPreviousMonthTxt(failures);
}

}  // namespace android_runtime_tests
