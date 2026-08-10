set(TIME_TRACKER_INFRA_INSIGHTS_EXPORT_SOURCES
    "insights/export_utils.module.cpp"
)

set(TIME_TRACKER_INFRA_INSIGHTS_DTO_SOURCES
    "insights/insights_dto_formatter.module.cpp"
)

set(TIME_TRACKER_INFRA_INSIGHTS_QUERYING_SOURCES
    "insights/lazy_sqlite_insights_query_service.module.cpp"
    "insights/insights_service.module.cpp"
    "insights/services/daily_insights_service.module.cpp"
    "insights/services/monthly_insights_service.module.cpp"
    "insights/services/weekly_insights_service.module.cpp"
    "insights/services/yearly_insights_service.module.cpp"
)

set(TIME_TRACKER_INFRA_INSIGHTS_DATA_QUERYING_SOURCES
    "insights/lazy_sqlite_insights_data_query_service.module.cpp"
    "insights/sqlite_insights_data_query_service.module.cpp"
)

set(TIME_TRACKER_INFRA_INSIGHTS_SOURCES
    ${TIME_TRACKER_INFRA_INSIGHTS_EXPORT_SOURCES}
    ${TIME_TRACKER_INFRA_INSIGHTS_DTO_SOURCES}
    ${TIME_TRACKER_INFRA_INSIGHTS_QUERYING_SOURCES}
    ${TIME_TRACKER_INFRA_INSIGHTS_DATA_QUERYING_SOURCES}
    "insights/facade/android_static_insights_formatter_registrar.cpp"
    "insights/facade/android_static_insights_formatter_registrar_support.cpp"
    "insights/facade/android_static_insights_formatter_registrar_builders.cpp"
    "insights/facade/insights_formatter_registry_adapter.cpp"
    "insights/daily/formatters/markdown/day_md_formatter_core.cpp"
    "insights/monthly/formatters/markdown/month_md_formatter_core.cpp"
    "insights/range/formatters/markdown/range_md_formatter_core.cpp"
)

if(TT_INSIGHTS_ENABLE_LATEX)
    list(APPEND TIME_TRACKER_INFRA_INSIGHTS_SOURCES
        "insights/daily/formatters/latex/day_tex_formatter_core.cpp"
        "insights/daily/formatters/latex/day_tex_utils.cpp"
        "insights/daily/formatters/statistics/latex_strategy.cpp"
        "insights/monthly/formatters/latex/month_tex_formatter_core.cpp"
        "insights/monthly/formatters/latex/month_tex_utils.cpp"
        "insights/range/formatters/latex/range_tex_formatter_core.cpp"
        "insights/range/formatters/latex/range_tex_utils.cpp"
    )
endif()

if(TT_INSIGHTS_ENABLE_TYPST)
    list(APPEND TIME_TRACKER_INFRA_INSIGHTS_SOURCES
        "insights/daily/formatters/typst/day_typ_formatter_core.cpp"
        "insights/daily/formatters/typst/day_typ_utils.cpp"
        "insights/daily/formatters/statistics/typst_strategy.cpp"
        "insights/monthly/formatters/typst/month_typ_formatter_core.cpp"
        "insights/range/formatters/typst/range_typ_formatter_core.cpp"
    )
endif()

set(TIME_TRACKER_INFRA_INSIGHTS_MODULE_FILES
    "insights/exporting/tracer.core.infrastructure.insights.exporting.cppm"
    "insights/exporting/tracer.core.infrastructure.insights.exporting.export_utils.cppm"
    "insights/dto/tracer.core.infrastructure.insights.dto.cppm"
    "insights/dto/tracer.core.infrastructure.insights.dto.formatter.cppm"
    "insights/data_querying/dq.cppm"
    "insights/data_querying/dq_lazy_sqlite.cppm"
    "insights/data_querying/dq_sqlite.cppm"
    "insights/querying/q.cppm"
    "insights/querying/q_lazy_sqlite.cppm"
    "insights/querying/q_insights_service.cppm"
    "insights/querying/services/svc.cppm"
    "insights/querying/services/day.cppm"
    "insights/querying/services/month.cppm"
    "insights/querying/services/week.cppm"
    "insights/querying/services/year.cppm"
)

set(TIME_TRACKER_INFRA_INSIGHTS_EXPORT_SOURCES ${TIME_TRACKER_INFRA_INSIGHTS_EXPORT_SOURCES})
set(TIME_TRACKER_INFRA_INSIGHTS_DTO_SOURCES ${TIME_TRACKER_INFRA_INSIGHTS_DTO_SOURCES})
set(TIME_TRACKER_INFRA_INSIGHTS_QUERYING_SOURCES ${TIME_TRACKER_INFRA_INSIGHTS_QUERYING_SOURCES})
set(TIME_TRACKER_INFRA_INSIGHTS_DATA_QUERYING_SOURCES ${TIME_TRACKER_INFRA_INSIGHTS_DATA_QUERYING_SOURCES})
set(TIME_TRACKER_INFRA_INSIGHTS_SOURCES ${TIME_TRACKER_INFRA_INSIGHTS_SOURCES})
