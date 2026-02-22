set(TIME_TRACKER_INFRA_REPORTS_SOURCES
    "reports/export_utils.cpp"
    "reports/exporter.cpp"
    "reports/report_file_manager.cpp"
    "reports/plugin_manifest.cpp"
    "reports/formatter_registry.cpp"
    "reports/facade/android_static_report_formatter_registrar.cpp"
    "reports/facade/report_formatter_registry_adapter.cpp"
    "reports/report_dto_export_writer.cpp"
    "reports/report_dto_formatter.cpp"
    "reports/daily/formatters/markdown/day_md_formatter_core.cpp"
    "reports/monthly/formatters/markdown/month_md_formatter_core.cpp"
    "reports/range/formatters/markdown/range_md_formatter_core.cpp"
    "reports/report_service.cpp"
    "reports/sqlite_report_data_query_service.cpp"
    "reports/services/daily_report_service.cpp"
    "reports/services/monthly_report_service.cpp"
    "reports/services/weekly_report_service.cpp"
    "reports/services/yearly_report_service.cpp"
    "reports/shared/factories/formatter_config_payload.cpp"
    "reports/shared/factories/formatter_config_payload_fillers.cpp"
    "reports/shared/factories/formatter_config_payload_cview.cpp"
    "reports/shared/factories/formatter_config_payload_lifecycle.cpp"
    "reports/shared/factories/generic_formatter_factory_resolver.cpp"
)

if(TT_REPORT_ENABLE_LATEX)
    list(APPEND TIME_TRACKER_INFRA_REPORTS_SOURCES
        "reports/daily/formatters/latex/day_tex_formatter_core.cpp"
        "reports/daily/formatters/latex/day_tex_utils.cpp"
        "reports/daily/formatters/statistics/latex_strategy.cpp"
        "reports/monthly/formatters/latex/month_tex_formatter_core.cpp"
        "reports/monthly/formatters/latex/month_tex_utils.cpp"
        "reports/range/formatters/latex/range_tex_formatter_core.cpp"
        "reports/range/formatters/latex/range_tex_utils.cpp"
    )
endif()

if(TT_REPORT_ENABLE_TYPST)
    list(APPEND TIME_TRACKER_INFRA_REPORTS_SOURCES
        "reports/daily/formatters/typst/day_typ_formatter_core.cpp"
        "reports/daily/formatters/typst/day_typ_utils.cpp"
        "reports/daily/formatters/statistics/typst_strategy.cpp"
        "reports/monthly/formatters/typst/month_typ_formatter_core.cpp"
        "reports/range/formatters/typst/range_typ_formatter_core.cpp"
    )
endif()
