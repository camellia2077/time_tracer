set(TIME_TRACKER_INFRA_REPORTING_EXPORT_SOURCES
    "reporting/export_utils.module.cpp"
    "reporting/exporter.module.cpp"
    "reporting/report_file_manager.module.cpp"
)

set(TIME_TRACKER_INFRA_REPORTING_DTO_SOURCES
    "reporting/report_dto_export_writer.module.cpp"
    "reporting/report_dto_formatter.module.cpp"
)

set(TIME_TRACKER_INFRA_REPORTING_QUERYING_SOURCES
    "reporting/lazy_sqlite_report_query_service.module.cpp"
    "reporting/report_service.module.cpp"
    "reporting/services/daily_report_service.module.cpp"
    "reporting/services/monthly_report_service.module.cpp"
    "reporting/services/weekly_report_service.module.cpp"
    "reporting/services/yearly_report_service.module.cpp"
)

set(TIME_TRACKER_INFRA_REPORTING_DATA_QUERYING_SOURCES
    "reporting/lazy_sqlite_report_data_query_service.module.cpp"
    "reporting/sqlite_report_data_query_service.module.cpp"
)

set(TIME_TRACKER_INFRA_REPORTING_SOURCES
    ${TIME_TRACKER_INFRA_REPORTING_EXPORT_SOURCES}
    ${TIME_TRACKER_INFRA_REPORTING_DTO_SOURCES}
    ${TIME_TRACKER_INFRA_REPORTING_QUERYING_SOURCES}
    ${TIME_TRACKER_INFRA_REPORTING_DATA_QUERYING_SOURCES}
    "reporting/facade/android_static_report_formatter_registrar.cpp"
    "reporting/facade/android_static_report_formatter_registrar_support.cpp"
    "reporting/facade/report_formatter_registry_adapter.cpp"
    "reporting/daily/formatters/markdown/day_md_formatter_core.cpp"
    "reporting/monthly/formatters/markdown/month_md_formatter_core.cpp"
    "reporting/range/formatters/markdown/range_md_formatter_core.cpp"
)

if(TT_REPORT_ENABLE_LATEX)
    list(APPEND TIME_TRACKER_INFRA_REPORTING_SOURCES
        "reporting/daily/formatters/latex/day_tex_formatter_core.cpp"
        "reporting/daily/formatters/latex/day_tex_utils.cpp"
        "reporting/daily/formatters/statistics/latex_strategy.cpp"
        "reporting/monthly/formatters/latex/month_tex_formatter_core.cpp"
        "reporting/monthly/formatters/latex/month_tex_utils.cpp"
        "reporting/range/formatters/latex/range_tex_formatter_core.cpp"
        "reporting/range/formatters/latex/range_tex_utils.cpp"
    )
endif()

if(TT_REPORT_ENABLE_TYPST)
    list(APPEND TIME_TRACKER_INFRA_REPORTING_SOURCES
        "reporting/daily/formatters/typst/day_typ_formatter_core.cpp"
        "reporting/daily/formatters/typst/day_typ_utils.cpp"
        "reporting/daily/formatters/statistics/typst_strategy.cpp"
        "reporting/monthly/formatters/typst/month_typ_formatter_core.cpp"
        "reporting/range/formatters/typst/range_typ_formatter_core.cpp"
    )
endif()

set(TIME_TRACKER_INFRA_REPORTING_MODULE_FILES
    "reporting/exporting/tracer.core.infrastructure.reporting.exporting.cppm"
    "reporting/exporting/tracer.core.infrastructure.reporting.exporting.export_utils.cppm"
    "reporting/exporting/tracer.core.infrastructure.reporting.exporting.report_file_manager.cppm"
    "reporting/exporting/tracer.core.infrastructure.reporting.exporting.exporter.cppm"
    "reporting/dto/tracer.core.infrastructure.reporting.dto.cppm"
    "reporting/dto/tracer.core.infrastructure.reporting.dto.formatter.cppm"
    "reporting/dto/tracer.core.infrastructure.reporting.dto.export_writer.cppm"
    "reporting/data_querying/dq.cppm"
    "reporting/data_querying/dq_lazy_sqlite.cppm"
    "reporting/data_querying/dq_sqlite.cppm"
    "reporting/querying/q.cppm"
    "reporting/querying/q_lazy_sqlite.cppm"
    "reporting/querying/q_report_service.cppm"
    "reporting/querying/services/svc.cppm"
    "reporting/querying/services/day.cppm"
    "reporting/querying/services/month.cppm"
    "reporting/querying/services/week.cppm"
    "reporting/querying/services/year.cppm"
)

set(TIME_TRACKER_INFRA_REPORTS_EXPORT_SOURCES ${TIME_TRACKER_INFRA_REPORTING_EXPORT_SOURCES})
set(TIME_TRACKER_INFRA_REPORTS_DTO_SOURCES ${TIME_TRACKER_INFRA_REPORTING_DTO_SOURCES})
set(TIME_TRACKER_INFRA_REPORTS_QUERYING_SOURCES ${TIME_TRACKER_INFRA_REPORTING_QUERYING_SOURCES})
set(TIME_TRACKER_INFRA_REPORTS_DATA_QUERYING_SOURCES ${TIME_TRACKER_INFRA_REPORTING_DATA_QUERYING_SOURCES})
set(TIME_TRACKER_INFRA_REPORTS_SOURCES ${TIME_TRACKER_INFRA_REPORTING_SOURCES})
