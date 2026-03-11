#include "infrastructure/tests/legacy_headers_compat/support.hpp"

auto TestLegacyPersistenceWriteHeaders(int& failures) -> void {
  using CanonicalSqliteTimeSheetRepository =
      tracer::core::infrastructure::persistence::SqliteTimeSheetRepository;
  using CanonicalRepository =
      tracer::core::infrastructure::persistence::importer::Repository;
  using CanonicalConnection =
      tracer::core::infrastructure::persistence::importer::sqlite::Connection;
  using CanonicalStatement =
      tracer::core::infrastructure::persistence::importer::sqlite::Statement;
  using CanonicalWriter =
      tracer::core::infrastructure::persistence::importer::sqlite::Writer;
  using CanonicalProjectResolver =
      tracer::core::infrastructure::persistence::importer::sqlite::
          ProjectResolver;

  using infrastructure::persistence::SqliteTimeSheetRepository;
  using infrastructure::persistence::importer::Repository;
  using infrastructure::persistence::importer::sqlite::Connection;
  using infrastructure::persistence::importer::sqlite::ProjectResolver;
  using infrastructure::persistence::importer::sqlite::Statement;
  using infrastructure::persistence::importer::sqlite::Writer;

  Expect(std::is_class_v<SqliteTimeSheetRepository>,
         "Legacy SqliteTimeSheetRepository header path should remain visible.",
         failures);
  Expect(std::is_class_v<Repository>,
         "Legacy importer::Repository header path should remain visible.",
         failures);
  Expect(std::is_class_v<Connection>,
         "Legacy sqlite::Connection header path should remain visible.",
         failures);
  Expect(std::is_class_v<Statement>,
         "Legacy sqlite::Statement header path should remain visible.",
         failures);
  Expect(std::is_class_v<Writer>,
         "Legacy sqlite::Writer header path should remain visible.", failures);
  Expect(std::is_class_v<ProjectResolver>,
         "Legacy sqlite::ProjectResolver header path should remain visible.",
         failures);

  Expect(std::is_class_v<CanonicalSqliteTimeSheetRepository>,
         "Canonical SqliteTimeSheetRepository header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalRepository>,
         "Canonical importer::Repository header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalConnection>,
         "Canonical sqlite::Connection header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalStatement>,
         "Canonical sqlite::Statement header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalWriter>,
         "Canonical sqlite::Writer header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalProjectResolver>,
         "Canonical sqlite::ProjectResolver header contract should be visible.",
         failures);

  const auto legacy_import_data = &Repository::ImportData;
  const auto legacy_replace_month_data = &Repository::ReplaceMonthData;
  const auto legacy_latest_tail =
      &Repository::TryGetLatestActivityTailBeforeDate;
  const auto legacy_execute_sql =
      &infrastructure::persistence::importer::sqlite::ExecuteSql;
  const auto canonical_import_data = &CanonicalRepository::ImportData;
  const auto canonical_replace_month_data =
      &CanonicalRepository::ReplaceMonthData;
  const auto canonical_latest_tail =
      &CanonicalRepository::TryGetLatestActivityTailBeforeDate;
  const auto canonical_execute_sql =
      &tracer::core::infrastructure::persistence::importer::sqlite::
          ExecuteSql;
  (void)legacy_import_data;
  (void)legacy_replace_month_data;
  (void)legacy_latest_tail;
  (void)legacy_execute_sql;
  (void)canonical_import_data;
  (void)canonical_replace_month_data;
  (void)canonical_latest_tail;
  (void)canonical_execute_sql;

  const std::filesystem::path legacy_repo_path =
      std::filesystem::path("temp") / "phase7_legacy_persistence_repo.sqlite";
  const std::filesystem::path canonical_repo_path =
      std::filesystem::path("temp") /
      "phase7_canonical_persistence_repo.sqlite";
  std::error_code cleanup_error;
  std::filesystem::remove(legacy_repo_path, cleanup_error);
  std::filesystem::remove(canonical_repo_path, cleanup_error);

  SqliteTimeSheetRepository legacy_repo(legacy_repo_path.string());
  CanonicalSqliteTimeSheetRepository canonical_repo(
      canonical_repo_path.string());
  Expect(!legacy_repo.IsDbOpen(),
         "Legacy SqliteTimeSheetRepository should remain side-effect free on construction.",
         failures);
  Expect(!canonical_repo.IsDbOpen(),
         "Canonical SqliteTimeSheetRepository should remain side-effect free on construction.",
         failures);

  std::filesystem::remove(legacy_repo_path, cleanup_error);
  std::filesystem::remove(canonical_repo_path, cleanup_error);
}

auto TestLegacyPersistenceRuntimeHeaders(int& failures) -> void {
  using CanonicalSqliteDatabaseHealthChecker =
      tracer::core::infrastructure::persistence::SqliteDatabaseHealthChecker;
  using CanonicalSqliteProjectRepository =
      tracer::core::infrastructure::persistence::SqliteProjectRepository;
  using LegacyNamespacedSqliteProjectRepository =
      infrastructure::persistence::SqliteProjectRepository;
  using infrastructure::persistence::SqliteDatabaseHealthChecker;

  Expect(std::is_class_v<SqliteProjectRepository>,
         "Legacy global SqliteProjectRepository header path should remain visible.",
         failures);
  Expect(std::is_class_v<LegacyNamespacedSqliteProjectRepository>,
         "Legacy namespaced SqliteProjectRepository header path should remain visible.",
         failures);
  Expect(std::is_class_v<SqliteDatabaseHealthChecker>,
         "Legacy SqliteDatabaseHealthChecker header path should remain visible.",
         failures);
  Expect(std::is_class_v<CanonicalSqliteProjectRepository>,
         "Canonical SqliteProjectRepository header contract should be visible.",
         failures);
  Expect(std::is_class_v<CanonicalSqliteDatabaseHealthChecker>,
         "Canonical SqliteDatabaseHealthChecker header contract should be visible.",
         failures);

  const auto legacy_get_all_projects = &SqliteProjectRepository::GetAllProjects;
  const auto legacy_namespaced_get_all_projects =
      &LegacyNamespacedSqliteProjectRepository::GetAllProjects;
  const auto canonical_get_all_projects =
      &CanonicalSqliteProjectRepository::GetAllProjects;
  const auto legacy_check_ready = &SqliteDatabaseHealthChecker::CheckReady;
  const auto canonical_check_ready =
      &CanonicalSqliteDatabaseHealthChecker::CheckReady;
  (void)legacy_get_all_projects;
  (void)legacy_namespaced_get_all_projects;
  (void)canonical_get_all_projects;
  (void)legacy_check_ready;
  (void)canonical_check_ready;

  const std::filesystem::path runtime_root =
      std::filesystem::path("temp") / "phase10_persistence_runtime_headers";
  const std::filesystem::path db_path = runtime_root / "runtime.sqlite";
  std::error_code cleanup_error;
  std::filesystem::remove_all(runtime_root, cleanup_error);
  std::filesystem::create_directories(runtime_root, cleanup_error);

  SqliteDatabaseHealthChecker legacy_health_checker(db_path.string());
  CanonicalSqliteDatabaseHealthChecker canonical_health_checker(db_path.string());
  Expect(legacy_health_checker.CheckReady().ok,
         "Legacy SqliteDatabaseHealthChecker should accept valid runtime path.",
         failures);
  Expect(canonical_health_checker.CheckReady().ok,
         "Canonical SqliteDatabaseHealthChecker should accept valid runtime path.",
         failures);

  try {
    infrastructure::persistence::importer::sqlite::Connection connection(
        db_path.string());
    const bool seeded =
        infrastructure::persistence::importer::sqlite::ExecuteSql(
            connection.GetDb(),
            "INSERT INTO projects "
            "(id, name, parent_id, full_path, depth) "
            "VALUES (1, 'Root', NULL, 'Root', 0);",
            "seed persistence runtime compatibility project");
    Expect(seeded,
           "Legacy sqlite::ExecuteSql should seed runtime project repository test db.",
           failures);

    SqliteProjectRepository legacy_repo(db_path.string());
    CanonicalSqliteProjectRepository canonical_repo(db_path.string());
    const auto legacy_projects = legacy_repo.GetAllProjects();
    const auto canonical_projects = canonical_repo.GetAllProjects();
    Expect(legacy_projects.size() == 1U && legacy_projects.front().name == "Root",
           "Legacy SqliteProjectRepository should read seeded project rows.",
           failures);
    Expect(canonical_projects.size() == 1U &&
               canonical_projects.front().name == "Root",
           "Canonical SqliteProjectRepository should read seeded project rows.",
           failures);
  } catch (...) {
    Expect(false,
           "Persistence runtime legacy/canonical adapters should construct and query successfully.",
           failures);
  }

  std::filesystem::remove_all(runtime_root, cleanup_error);
}
