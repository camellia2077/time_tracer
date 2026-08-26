import tracer.core.infrastructure.persistence.runtime;
import tracer.core.infrastructure.persistence.write;

#include "infra/tests/modules_smoke/persistence_runtime.hpp"
#include "application/pipeline/importer/model/import_models.hpp"

#include <filesystem>

namespace {

auto RunPersistenceRuntimeSmokeImpl() -> int {
  std::error_code cleanup_error;

  const auto kCheckReady = &tracer::core::infrastructure::persistence::
                               SqliteDatabaseHealthChecker::CheckReady;
  const auto kGetAllProjects = &tracer::core::infrastructure::persistence::
                                   SqliteProjectRepository::GetAllProjects;
  const auto kListIngestSyncStatuses =
      &tracer::core::infrastructure::persistence::
          SqliteIngestRuntimeRepository::ListIngestSyncStatuses;
  (void)kCheckReady;
  (void)kGetAllProjects;
  (void)kListIngestSyncStatuses;

  const std::filesystem::path kPersistenceRuntimeSmokeDir =
      std::filesystem::path("temp") / "phase10_infra_module_smoke";
  std::filesystem::create_directories(kPersistenceRuntimeSmokeDir);
  const std::filesystem::path kRuntimeDbPath =
      kPersistenceRuntimeSmokeDir / "persistence_runtime.sqlite";
  std::filesystem::remove(kRuntimeDbPath, cleanup_error);

  tracer::core::infrastructure::persistence::SqliteDatabaseHealthChecker
      database_health_checker(kRuntimeDbPath.string());
  if (!database_health_checker.CheckReady().ok) {
    return 26;
  }

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        runtime_connection(kRuntimeDbPath.string());
    if (runtime_connection.GetDb() == nullptr) {
      return 27;
    }
    if (!tracer::core::infrastructure::persistence::importer::sqlite::
            ExecuteSql(runtime_connection.GetDb(),
                       "INSERT INTO projects "
                       "(id, name, parent_id, full_path, depth) "
                       "VALUES (1, 'Root', NULL, 'Root', 0);",
                       "seed persistence runtime smoke project")) {
      return 28;
    }

    tracer::core::infrastructure::persistence::SqliteProjectRepository
        project_repository(kRuntimeDbPath.string());
    const auto projects = project_repository.GetAllProjects();
    if (projects.size() != 1U || projects.front().name != "Root") {
      return 29;
    }

    tracer::core::infrastructure::persistence::SqliteIngestRuntimeRepository
        ingest_runtime_repository(kRuntimeDbPath.string());
    const auto sync_status =
        ingest_runtime_repository.ListIngestSyncStatuses({});
    if (!sync_status.ok || !sync_status.items.empty()) {
      return 31;
    }

    tracer::core::infrastructure::persistence::importer::sqlite::Statement
        statement(runtime_connection.GetDb());
    tracer::core::infrastructure::persistence::importer::sqlite::Writer writer(
        runtime_connection.GetDb(), statement.GetInsertDayStmt(),
        statement.GetInsertRecordStmt(), statement.GetInsertProjectStmt());
    writer.InsertDays({
        DayData{.date = "2026-02-01",
                .remark = "",
                .getup_time = std::nullopt,
                .year = 2026,
                .month = 2,
                .activity_count = 1},
        DayData{.date = "2026-02-02",
                .remark = "",
                .getup_time = std::nullopt,
                .year = 2026,
                .month = 2,
                .activity_count = 2},
        DayData{.date = "2026-02-03",
                .remark = "",
                .getup_time = std::nullopt,
                .year = 2026,
                .month = 2,
                .activity_count = 2},
    });
    writer.InsertRecords({
        TimeRecordInternal{.kind = ActivityRecordKind::kEndOnly,
                           .logical_id = 10,
                           .start_timestamp = 0,
                           .end_timestamp = 1770000000,
                           .start_time_str = "",
                           .end_time_str = "12:00:00",
                           .project_path = "work",
                           .duration_seconds = 0,
                           .remark = std::nullopt,
                           .date = "2026-02-01"},
        TimeRecordInternal{.kind = ActivityRecordKind::kInterval,
                           .logical_id = 20,
                           .start_timestamp = 1770000100,
                           .end_timestamp = 1770003600,
                           .start_time_str = "10:00:00",
                           .end_time_str = "11:00:00",
                           .project_path = "study",
                           .duration_seconds = 3600,
                           .remark = std::nullopt,
                           .date = "2026-02-02"},
        TimeRecordInternal{.kind = ActivityRecordKind::kInterval,
                           .logical_id = 21,
                           .start_timestamp = 1770003700,
                           .end_timestamp = 1770007200,
                           .start_time_str = "11:00:00",
                           .end_time_str = "12:00:00",
                           .project_path = "exercise",
                           .duration_seconds = 3600,
                           .remark = std::nullopt,
                           .date = "2026-02-02"},
        // The logical day continues after midnight. Natural-day timestamps
        // would put this activity before the preceding 23:00 one, but its
        // higher logical_id makes it the final activity of this logical day.
        TimeRecordInternal{.kind = ActivityRecordKind::kInterval,
                           .logical_id = 30,
                           .start_timestamp = 1770098400,
                           .end_timestamp = 1770102000,
                           .start_time_str = "22:00:00",
                           .end_time_str = "23:00:00",
                           .project_path = "evening",
                           .duration_seconds = 3600,
                           .remark = std::nullopt,
                           .date = "2026-02-03"},
        TimeRecordInternal{.kind = ActivityRecordKind::kInterval,
                           .logical_id = 31,
                           .start_timestamp = 1770022800,
                           .end_timestamp = 1770026400,
                           .start_time_str = "01:00:00",
                           .end_time_str = "02:00:00",
                           .project_path = "after_midnight",
                           .duration_seconds = 3600,
                           .remark = std::nullopt,
                           .date = "2026-02-03"},
    });

    const auto previous_tail =
        ingest_runtime_repository.TryGetLatestActivityTailBeforeDate(
            "2026-02-02");
    if (!previous_tail.has_value() || previous_tail->date != "2026-02-01" ||
        previous_tail->end_time != "12:00:00") {
      return 32;
    }

    const auto inclusive_tail =
        ingest_runtime_repository.TryGetLatestActivityTailAtOrBeforeDate(
            "2026-02-02");
    if (!inclusive_tail.has_value() || inclusive_tail->date != "2026-02-02" ||
        inclusive_tail->end_time != "12:00:00") {
      return 33;
    }

    const auto latest_record =
        ingest_runtime_repository.TryGetLatestActivityRecordOnDate(
            "2026-02-02");
    if (!latest_record.has_value() || latest_record->activity != "exercise" ||
        latest_record->record_kind != "interval" ||
        latest_record->start_time != "11:00:00" ||
        latest_record->end_time != "12:00:00" ||
        latest_record->duration_seconds != 3600) {
      return 34;
    }

    const auto latest_cross_midnight_record =
        ingest_runtime_repository.TryGetLatestActivityRecordOnDate(
            "2026-02-03");
    if (!latest_cross_midnight_record.has_value() ||
        latest_cross_midnight_record->activity != "after_midnight" ||
        latest_cross_midnight_record->start_time != "01:00:00" ||
        latest_cross_midnight_record->end_time != "02:00:00") {
      return 35;
    }
  } catch (...) {
    return 30;
  }

  std::filesystem::remove(kRuntimeDbPath, cleanup_error);
  std::filesystem::remove_all(kPersistenceRuntimeSmokeDir, cleanup_error);
  return 0;
}

}  // namespace

auto RunInfrastructureModulePersistenceRuntimeSmoke() -> int {
  return RunPersistenceRuntimeSmokeImpl();
}
