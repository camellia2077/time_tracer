# 模块1：源文件收集 (SourceFileCollection.cmake)
# 将所有 file(GLOB ...) 命令集中到这里

file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB TIME_MASTER_CLI_SOURCES 
    "time_master_cli/*.cpp"
)
file(GLOB TIME_MASTER_APP_SOURCES 
    "time_master_app/*.cpp"
    "time_master_app/menu_processor/*.cpp"
    "time_master_app/menu_input/*.cpp"
)
file(GLOB DB_INSERTER_SOURCES
    "db_inserter/*.cpp"
    "db_inserter/model/*.cpp"
    "db_inserter/inserter/*.cpp"
    "db_inserter/parser/*.cpp"
    "db_inserter/parser/internal/*.cpp"
)
file(GLOB QUERIES_SOURCES
    "queries/*.cpp"
    "queries/export/*.cpp"
    "queries/daily/*.cpp"
    "queries/daily/formatters/*.cpp"
    "queries/daily/formatters/md/*.cpp"
    "queries/daily/formatters/tex/*.cpp"
    "queries/daily/formatters/typ/*.cpp"
    "queries/monthly/*.cpp"
    "queries/monthly/formatters/*.cpp"
    "queries/monthly/formatters/md/*.cpp"
    "queries/monthly/formatters/tex/*.cpp"
    "queries/monthly/formatters/typ/*.cpp"
    "queries/period/*.cpp"
    "queries/period/formatters/*.cpp"
    "queries/period/formatters/md/*.cpp"
    "queries/period/formatters/tex/*.cpp"
    "queries/period/formatters/typ/*.cpp"
    "queries/shared/*.cpp"
    "queries/shared/breakdown/*.cpp"
    "queries/shared/breakdown/formatters/*.cpp"
)
file(GLOB REPROCESSING_SOURCES
    "reprocessing/*.cpp"
    "reprocessing/Converter/*.cpp"
    "reprocessing/Converter/internal/*.cpp"
    "reprocessing/Converter/model/*.cpp"
    "reprocessing/validator/*.cpp"
    "reprocessing/validator/internal/*.cpp"
)
file(GLOB FILE_HANDLER_SOURCES "file_handler/*.cpp")
file(GLOB ACTION_HANDLER_SOURCES 
    "action_handler/*.cpp"
    "action_handler/database/*.cpp"
    "action_handler/file/*.cpp"
    "action_handler/query/*.cpp"
    "action_handler/reporting/*.cpp"
)