set(TIME_TRACKER_INFRA_PERSISTENCE_WRITE_SOURCES
    "persistence/sqlite_time_sheet_repository.module.cpp"
    "persistence/importer/repository.module.cpp"
    "persistence/importer/sqlite/writer.module.cpp"
    "persistence/importer/sqlite/project_resolver.module.cpp"
    "persistence/importer/sqlite/connection.module.cpp"
    "persistence/importer/sqlite/statement.module.cpp"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_SOURCES
    "persistence/repositories/sqlite_project_repository.module.cpp"
    "persistence/sqlite_database_health_checker.module.cpp"
)

set(TIME_TRACKER_INFRA_PERSISTENCE_SOURCES
    "persistence/sqlite/db_manager.cpp"
    "persistence/sqlite_data_query_service.cpp"
    "persistence/sqlite_data_query_service_request.cpp"
    "persistence/sqlite_data_query_service_report_mapping.cpp"
    "persistence/sqlite_data_query_service_dispatch.cpp"
    "persistence/sqlite_data_query_service_period.cpp"
    ${TIME_TRACKER_INFRA_PERSISTENCE_RUNTIME_SOURCES}
    ${TIME_TRACKER_INFRA_PERSISTENCE_WRITE_SOURCES}
)
