# 模块1：源文件收集 (SourceFileCollection.cmake)
# ----------------------------------------------------
# 最佳实践：显式列出所有源文件以确保构建系统的稳定性和可预测性。

# --- Common Sources ---
set(COMMON_SOURCES
    "src/common/common_utils.cpp"
)

# --- Time Master CLI Sources ---
set(TIME_MASTER_CLI_SOURCES
    "src/time_master_cli/CliController.cpp"
    "src/time_master_cli/CliParser.cpp"
)

# --- DB Inserter Sources ---
set(DB_INSERTER_SOURCES
    "src/db_inserter/DataImporter.cpp"
    # 插入的封装
    "src/db_inserter/inserter/facade/DatabaseInserter.cpp"
    # 插入的实现
    "src/db_inserter/inserter/_internal/DataInserter.cpp"
    "src/db_inserter/inserter/_internal/DbConnectionManager"
    "src/db_inserter/inserter/_internal/DbStatementManager.cpp"

    


    # 解析json
    "src/db_inserter/parser/JsonDataParser.cpp"

)

# --- Queries Sources ---
set(QUERIES_SOURCES
    # Root
    "src/queries/QueryHandler.cpp"
    # Export
    "src/queries/export/AllDayReports.cpp"
    "src/queries/export/AllMonthlyReports.cpp"
    "src/queries/export/AllPeriodReports.cpp"
    # Daily Reports
    "src/queries/daily/DayGenerator.cpp"
    "src/queries/daily/DayQuerier.cpp"
    "src/queries/daily/formatters/md/DayMd.cpp"
    "src/queries/daily/formatters/tex/DayTex.cpp"
    "src/queries/daily/formatters/typ/DayTyp.cpp"
    # Monthly Reports
    "src/queries/monthly/MonthGenerator.cpp"
    "src/queries/monthly/MonthQuerier.cpp"
    "src/queries/monthly/formatters/md/MonthMd.cpp"
    "src/queries/monthly/formatters/tex/MonthTex.cpp"
    "src/queries/monthly/formatters/typ/MonthTyp.cpp"
    # Period Reports
    "src/queries/period/PeriodGenerator.cpp"
    "src/queries/period/PeriodQuerier.cpp"
    "src/queries/period/formatters/md/PeriodMd.cpp"
    "src/queries/period/formatters/tex/PeriodTex.cpp"
    "src/queries/period/formatters/typ/PeriodTyp.cpp"
    # Shared Components
    "src/queries/shared/factories/TreeFmtFactory.cpp"
    "src/queries/shared/formatters/breakdown/md/BreakdownMd.cpp"
    "src/queries/shared/formatters/breakdown/tex/BreakdownTex.cpp"
    "src/queries/shared/formatters/breakdown/typ/BreakdownTyp.cpp"
    "src/queries/shared/utils/query_utils.cpp"
)

# --- Reprocessing Sources ---
set(REPROCESSING_SOURCES
    "src/reprocessing/LogProcessor.cpp"

    "src/reprocessing/converter/IntervalConverter.cpp"

    "src/reprocessing/converter/internal/Converter.cpp"
    "src/reprocessing/converter/internal/ConverterConfig.cpp"
    "src/reprocessing/converter/internal/DayProcessor.cpp"
    "src/reprocessing/converter/internal/InputParser.cpp"
    "src/reprocessing/converter/internal/OutputGenerator.cpp"

    # 验证模块
    "src/reprocessing/validator/FileValidator.cpp"

    "src/reprocessing/validator/common/ValidatorUtils.cpp"

    # 验证封装
    "src/reprocessing/validator/output_json/facade/JsonValidator.cpp"
    # 验证实现
    "src/reprocessing/validator/output_json/_internal/JsonValidatorActivities.cpp"
    "src/reprocessing/validator/output_json/_internal/JsonValidatorDate.cpp"
    "src/reprocessing/validator/output_json/_internal/JsonValidatorRules.cpp"
    "src/reprocessing/validator/output_json/_internal/JsonValidatorTime.cpp"

    "src/reprocessing/validator/source_txt/SourceFileValidator.cpp"

)

# --- File Handler Sources ---
set(FILE_HANDLER_SOURCES
    "src/file_handler/ConfigLoader.cpp"
    "src/file_handler/FileController.cpp"
    "src/file_handler/FileUtils.cpp"
)

# --- Action Handler Sources ---
set(ACTION_HANDLER_SOURCES
    "src/action_handler/FileProcessingHandler.cpp"
    "src/action_handler/ReportGenerationHandler.cpp"
    "src/action_handler/database/DatabaseManager.cpp"
    "src/action_handler/file/FilePipelineManager.cpp"
    "src/action_handler/query/DirectQueryManager.cpp"
    "src/action_handler/reporting/ExportUtils.cpp"
    "src/action_handler/reporting/ReportExporter.cpp"
)

message(STATUS "Source files collected explicitly.")