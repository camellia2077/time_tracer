# 模块1：源文件收集 (SourceFileCollection.cmake)
# 将所有 file(GLOB ...) 命令集中到这里

file(GLOB COMMON_SOURCES "src/common/*.cpp")
file(GLOB TIME_MASTER_CLI_SOURCES
    "src/time_master_cli/*.cpp"
)
file(GLOB TIME_MASTER_APP_SOURCES
    "src/time_master_app/*.cpp"
    "src/time_master_app/menu_processor/*.cpp"
    "src/time_master_app/menu_export/*.cpp"
    "src/time_master_app/menu_query/*.cpp"
    "src/time_master_app/menu_input/*.cpp"
)
file(GLOB DB_INSERTER_SOURCES
    "src/db_inserter/*.cpp"
    "src/db_inserter/model/*.cpp"
    "src/db_inserter/inserter/*.cpp"
    "src/db_inserter/parser/*.cpp"
    "src/db_inserter/parser/internal/*.cpp"
)
file(GLOB QUERIES_SOURCES
    "src/queries/*.cpp"
    "src/queries/export/*.cpp"
    "src/queries/daily/*.cpp"
    "src/queries/daily/formatters/*.cpp"
    "src/queries/daily/formatters/md/*.cpp"
    "src/queries/daily/formatters/tex/*.cpp"
    "src/queries/daily/formatters/typ/*.cpp"
    "src/queries/monthly/*.cpp"
    "src/queries/monthly/formatters/*.cpp"
    "src/queries/monthly/formatters/md/*.cpp"
    "src/queries/monthly/formatters/tex/*.cpp"
    "src/queries/monthly/formatters/typ/*.cpp"
    "src/queries/period/*.cpp"
    "src/queries/period/formatters/*.cpp"
    "src/queries/period/formatters/md/*.cpp"
    "src/queries/period/formatters/tex/*.cpp"
    "src/queries/period/formatters/typ/*.cpp"
    "src/queries/shared/*.cpp"
    "src/queries/shared/breakdown/*.cpp"
    "src/queries/shared/breakdown/formatters/*.cpp"
)
file(GLOB REPROCESSING_SOURCES
    "src/reprocessing/*.cpp"
    "src/reprocessing/Converter/*.cpp"
    "src/reprocessing/Converter/internal/*.cpp"
    "src/reprocessing/Converter/model/*.cpp"
    "src/reprocessing/validator/*.cpp"
    "src/reprocessing/validator/internal/*.cpp"
)
file(GLOB FILE_HANDLER_SOURCES "src/file_handler/*.cpp")
file(GLOB ACTION_HANDLER_SOURCES
    "src/action_handler/*.cpp"
    "src/action_handler/database/*.cpp"
    "src/action_handler/file/*.cpp"
    "src/action_handler/query/*.cpp"
    "src/action_handler/reporting/*.cpp"
)