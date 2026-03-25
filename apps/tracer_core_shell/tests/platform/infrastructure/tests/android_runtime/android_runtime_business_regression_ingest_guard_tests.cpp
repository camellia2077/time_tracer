// infrastructure/tests/android_runtime/android_runtime_business_regression_ingest_guard_tests.cpp
#include <sqlite3.h>

#include <exception>
#include <filesystem>
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_business_regression_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests::business_regression_internal {
namespace {

auto TestSuccessfulIngestCreatesDbAndPersistsData(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_ingest_success_creates_db_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path input_path =
        paths.test_root / "input" / "ok.txt";
    if (!WriteFileWithParents(input_path, BuildValidSingleMonthTxt())) {
      ++failures;
      std::cerr << "[FAIL] Failed to create valid ingest fixture.\n";
      RemoveTree(paths.test_root);
      return;
    }

    if (std::filesystem::exists(paths.db_path)) {
      ++failures;
      std::cerr << "[FAIL] Successful-ingest fixture should start without a "
                   "database file.\n";
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (!ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Valid ingest should succeed: "
                << ingest_result.error_message << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    if (!std::filesystem::exists(paths.db_path)) {
      ++failures;
      std::cerr << "[FAIL] Successful ingest should create the database "
                   "file.\n";
      RemoveTree(paths.test_root);
      return;
    }

    sqlite3* database = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed after successful "
                   "ingest.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const auto day_count = QueryCount(database, "SELECT COUNT(*) FROM days;");
    const auto record_count =
        QueryCount(database, "SELECT COUNT(*) FROM time_records;");
    sqlite3_close(database);

    if (!day_count.has_value() || *day_count <= 0) {
      ++failures;
      std::cerr << "[FAIL] Successful ingest should persist at least one day "
                   "row.\n";
    }
    if (!record_count.has_value() || *record_count <= 0) {
      ++failures;
      std::cerr << "[FAIL] Successful ingest should persist at least one "
                   "time_records row.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Successful ingest persistence test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Successful ingest persistence test threw "
                 "non-standard exception.\n";
  }

  RemoveTree(paths.test_root);
}

auto TestInvalidStructureIngestDoesNotCreateDb(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_invalid_structure_no_db_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path input_path =
        paths.test_root / "input" / "invalid_structure.txt";
    if (!WriteFileWithParents(input_path,
                              BuildInvalidStructureSingleMonthTxt())) {
      ++failures;
      std::cerr << "[FAIL] Failed to create invalid-structure ingest "
                   "fixture.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Invalid-structure ingest should fail.\n";
    }
    ExpectNoDbArtifacts(paths.db_path, "Invalid-structure ingest", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Invalid-structure no-db test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Invalid-structure no-db test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

auto TestInvalidLogicIngestDoesNotCreateDb(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_invalid_logic_no_db_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path input_path =
        paths.test_root / "input" / "invalid_logic.txt";
    if (!WriteFileWithParents(input_path, BuildInvalidLogicSingleMonthTxt())) {
      ++failures;
      std::cerr << "[FAIL] Failed to create invalid-logic ingest fixture.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kContinuity;
    const auto ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Invalid-logic ingest should fail.\n";
    }
    ExpectNoDbArtifacts(paths.db_path, "Invalid-logic ingest", failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Invalid-logic no-db test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Invalid-logic no-db test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

auto TestReplaceMonthInvalidInputDoesNotCreateDb(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_replace_month_invalid_no_db_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path input_path =
        paths.test_root / "input" / "replace_invalid.txt";
    if (!WriteFileWithParents(input_path,
                              BuildInvalidStructureSingleMonthTxt())) {
      ++failures;
      std::cerr << "[FAIL] Failed to create replace-month invalid ingest "
                   "fixture.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    ingest_request.ingest_mode = IngestMode::kSingleTxtReplaceMonth;
    const auto ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Replace-month ingest should fail for invalid "
                   "input.\n";
    }
    ExpectNoDbArtifacts(paths.db_path, "Invalid replace-month ingest",
                        failures);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Replace-month invalid no-db test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Replace-month invalid no-db test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

auto TestFailedIngestDoesNotMutateExistingDb(int& failures) -> void {
  const RuntimeTestPaths paths =
      BuildTempTestPaths("time_tracer_failed_ingest_existing_db_test");
  const std::filesystem::path kConfigTomlPath =
      BuildRepoRoot() / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  RemoveTree(paths.test_root);

  try {
    const auto request = BuildRuntimeRequest(paths, kConfigTomlPath);
    auto runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
    if (!runtime.runtime_api) {
      ++failures;
      std::cerr
          << "[FAIL] BuildAndroidRuntime should return a valid runtime API.\n";
      RemoveTree(paths.test_root);
      return;
    }

    const std::filesystem::path ok_input_path =
        paths.test_root / "input" / "ok.txt";
    const std::filesystem::path invalid_input_path =
        paths.test_root / "input" / "invalid_structure.txt";
    if (!WriteFileWithParents(ok_input_path, BuildValidSingleMonthTxt()) ||
        !WriteFileWithParents(invalid_input_path,
                              BuildInvalidStructureSingleMonthTxt())) {
      ++failures;
      std::cerr << "[FAIL] Failed to create existing-db mutation fixtures.\n";
      RemoveTree(paths.test_root);
      return;
    }

    tracer_core::core::dto::IngestRequest ingest_request;
    ingest_request.input_path = ok_input_path.string();
    ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto first_ingest_result =
        runtime.runtime_api->pipeline().RunIngest(ingest_request);
    if (!first_ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Baseline ingest should succeed before mutation "
                   "check: "
                << first_ingest_result.error_message << '\n';
      RemoveTree(paths.test_root);
      return;
    }

    sqlite3* database = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed for existing-db "
                   "mutation check.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const auto before_day_count =
        QueryCount(database, "SELECT COUNT(*) FROM days;");
    const auto before_record_count =
        QueryCount(database, "SELECT COUNT(*) FROM time_records;");
    sqlite3_close(database);

    tracer_core::core::dto::IngestRequest invalid_ingest_request;
    invalid_ingest_request.input_path = invalid_input_path.string();
    invalid_ingest_request.date_check_mode = DateCheckMode::kNone;
    const auto invalid_ingest_result =
        runtime.runtime_api->pipeline().RunIngest(invalid_ingest_request);
    if (invalid_ingest_result.ok) {
      ++failures;
      std::cerr << "[FAIL] Invalid ingest against existing DB should fail.\n";
      RemoveTree(paths.test_root);
      return;
    }

    database = nullptr;
    if (sqlite3_open(paths.db_path.string().c_str(), &database) != SQLITE_OK ||
        database == nullptr) {
      ++failures;
      std::cerr << "[FAIL] sqlite3_open should succeed after failed ingest on "
                   "existing DB.\n";
      if (database != nullptr) {
        sqlite3_close(database);
      }
      RemoveTree(paths.test_root);
      return;
    }

    const auto after_day_count =
        QueryCount(database, "SELECT COUNT(*) FROM days;");
    const auto after_record_count =
        QueryCount(database, "SELECT COUNT(*) FROM time_records;");
    sqlite3_close(database);

    if (!before_day_count.has_value() || !after_day_count.has_value() ||
        before_day_count != after_day_count) {
      ++failures;
      std::cerr << "[FAIL] Failed ingest should not change persisted day "
                   "row count.\n";
    }
    if (!before_record_count.has_value() || !after_record_count.has_value() ||
        before_record_count != after_record_count) {
      ++failures;
      std::cerr << "[FAIL] Failed ingest should not change persisted "
                   "time_records row count.\n";
    }
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] Existing-db mutation guard test threw exception: "
              << exception.what() << '\n';
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Existing-db mutation guard test threw non-standard "
                 "exception.\n";
  }

  RemoveTree(paths.test_root);
}

}  // namespace

auto RunBusinessRegressionIngestGuardTests(int& failures) -> void {
  TestSuccessfulIngestCreatesDbAndPersistsData(failures);
  TestInvalidStructureIngestDoesNotCreateDb(failures);
  TestInvalidLogicIngestDoesNotCreateDb(failures);
  TestReplaceMonthInvalidInputDoesNotCreateDb(failures);
  TestFailedIngestDoesNotMutateExistingDb(failures);
}

}  // namespace android_runtime_tests::business_regression_internal
