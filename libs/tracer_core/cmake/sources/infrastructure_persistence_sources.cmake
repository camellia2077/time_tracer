set(TIME_TRACKER_INFRA_PERSISTENCE_SUPPORT_SOURCES
    "persistence/sqlite/db_manager.cpp"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_WRITE_SOURCES
    "persistence/sqlite_time_sheet_repository.module.cpp"
    "persistence/importer/repository.module.cpp"
    "persistence/importer/repository_ingest_sync_sql.cpp"
    "persistence/importer/sqlite/writer.module.cpp"
    "persistence/importer/sqlite/project_resolver.module.cpp"
    "persistence/importer/sqlite/connection.module.cpp"
    "persistence/importer/sqlite/statement.module.cpp"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_SOURCES
    "persistence/repositories/sqlite_project_repository.module.cpp"
    "persistence/sqlite_database_health_checker.module.cpp"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_ALL_SOURCES
    ${TIME_TRACKER_INFRA_PERSISTENCE_SUPPORT_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_SOURCES}
)

set(TIME_TRACKER_INFRA_PERSISTENCE_WRITE_MODULE_FILES
    "persistence/write/tracer.core.infrastructure.persistence.write.cppm"
    "persistence/write/tracer.core.infrastructure.persistence.write.sqlite_time_sheet_repository.cppm"
    "persistence/write/importer/imp.cppm"
    "persistence/write/importer/imp_repo.cppm"
    "persistence/write/importer/sqlite/sql.cppm"
    "persistence/write/importer/sqlite/sql_conn.cppm"
    "persistence/write/importer/sqlite/sql_stmt.cppm"
    "persistence/write/importer/sqlite/sql_writer.cppm"
    "persistence/write/importer/sqlite/sql_proj.cppm"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_MODULE_FILES
    "persistence/runtime/tracer.core.infrastructure.persistence.runtime.cppm"
    "persistence/runtime/rt_project_repo.cppm"
    "persistence/runtime/rt_db_health.cppm"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_SOURCES
    ${TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_ALL_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_WRITE_SOURCES}
)
