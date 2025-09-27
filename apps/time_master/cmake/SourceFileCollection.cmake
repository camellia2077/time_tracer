# 模块1：源文件收集 (SourceFileCollection.cmake)
# ----------------------------------------------------
# 最佳实践：显式列出所有源文件以确保构建系统的稳定性和可预测性。

# --- Common  ---
set(COMMON_SOURCES
    "src/common/utils/StringUtils.cpp"
    "src/common/utils/TimeUtils.cpp"
)


set(CONFIG_VALIDATOR
    # 顶层外观
    "src/config_validator/facade/ConfigFacade.cpp"

    # Reprocessing 领域的 Facade 和 Pipelines
    "src/config_validator/reprocessing/facade/ReprocFacade.cpp"
    "src/config_validator/reprocessing/rules/DurationRule.cpp"
    "src/config_validator/reprocessing/rules/MainRule.cpp"
    "src/config_validator/reprocessing/rules/MappingRule.cpp"

    # reports 领域的 Facade
    "src/config_validator/reports/facade/QueryFacade.cpp"

    # reports 领域的 Strategies (核心)
    "src/config_validator/reports/strategies/BaseStrategy.cpp"
    "src/config_validator/reports/strategies/StrategyFactory.cpp"

    # reports 领域的 Strategies (具体实现)
    "src/config_validator/reports/strategies/daily/DailyMd.cpp"
    "src/config_validator/reports/strategies/daily/DailyTex.cpp"
    "src/config_validator/reports/strategies/daily/DailyTyp.cpp"
    "src/config_validator/reports/strategies/monthly/Monthly.cpp"
    "src/config_validator/reports/strategies/periodic/Periodic.cpp"
)

# --- Time Master CLI Sources ---
set(TIME_MASTER_CLI_SOURCES
    "src/time_master_cli/CliController.cpp"
    "src/time_master_cli/CliParser.cpp"

    # Add all new command implementation files
    "src/time_master_cli/commands/export/Export.cpp"

    "src/time_master_cli/commands/query/Query.cpp"

    "src/time_master_cli/commands/pipeline/Convert.cpp"
    "src/time_master_cli/commands/pipeline/Import.cpp"
    "src/time_master_cli/commands/pipeline/Run.cpp"
    "src/time_master_cli/commands/pipeline/ValidateOutput.cpp"
    "src/time_master_cli/commands/pipeline/ValidateSource.cpp"


   
    
    
    
    

)
# --- DB Inserter Sources ---
set(DB_INSERTER_SOURCES
    "src/db_inserter/DataImporter.cpp"
    # 插入的封装
    "src/db_inserter/inserter/facade/DatabaseInserter.cpp"
    # 插入的实现
    "src/db_inserter/inserter/pipelines/DataInserter.cpp"
    "src/db_inserter/inserter/pipelines/DbConnectionManager.cpp"
    "src/db_inserter/inserter/pipelines/DbStatementManager.cpp"
    

    # 解析json
    "src/db_inserter/parser/facade/JsonParserFacade.cpp"
    "src/db_inserter/parser/pipelines/ActivityParser.cpp"
    "src/db_inserter/parser/pipelines/DayParser.cpp"
    

)

set(REPORTS_SOURCES
    # Root
    "src/reports/QueryHandler.cpp"
    # Export
    "src/reports/export/AllDayReports.cpp"
    "src/reports/export/AllMonthlyReports.cpp"
    "src/reports/export/AllPeriodReports.cpp"
    # Daily Reports
    
    "src/reports/daily/DayGenerator.cpp"
    "src/reports/daily/DayQuerier.cpp"
    # Daily base
    "src/reports/daily/formatters/base/DayBaseConfig.cpp"

    # Daily md
    

    "src/reports/daily/formatters/md/DayMd.cpp"
    "src/reports/daily/formatters/md/DayMdConfig.cpp"

    "src/reports/daily/formatters/tex/DayTex.cpp"
    "src/reports/daily/formatters/tex/DayTexConfig.cpp"
    "src/reports/daily/formatters/tex/DayTexUtils.cpp"

    "src/reports/daily/formatters/typ/DayTyp.cpp"
    "src/reports/daily/formatters/typ/DayTypConfig.cpp"
    "src/reports/daily/formatters/typ/DayTypUtils.cpp"
    # Monthly Reports
    "src/reports/monthly/MonthGenerator.cpp"
    "src/reports/monthly/MonthQuerier.cpp"
    "src/reports/monthly/formatters/base/MonthBaseConfig.cpp"
    # Monthly md
    "src/reports/monthly/formatters/md/MonthMd.cpp"
    "src/reports/monthly/formatters/md/MonthMdConfig.cpp"

    # Monthly tex
    "src/reports/monthly/formatters/tex/MonthTex.cpp"
    "src/reports/monthly/formatters/tex/MonthTexUtils.cpp"
    "src/reports/monthly/formatters/tex/MonthTexConfig.cpp"

    "src/reports/monthly/formatters/typ/MonthTyp.cpp"
    "src/reports/monthly/formatters/typ/MonthTypConfig.cpp"


    # Period Reports
    "src/reports/period/PeriodGenerator.cpp"
    "src/reports/period/PeriodQuerier.cpp"

    "src/reports/period/formatters/base/PeriodBaseConfig.cpp"

    # Period md
    "src/reports/period/formatters/md/PeriodMd.cpp"
    "src/reports/period/formatters/md/PeriodMdConfig.cpp"

    # Period tex
    "src/reports/period/formatters/tex/PeriodTex.cpp"
    "src/reports/period/formatters/tex/PeriodTexUtils.cpp"
    "src/reports/period/formatters/tex/PeriodTexConfig.cpp"

    # Period typ
    "src/reports/period/formatters/typ/PeriodTyp.cpp"
    "src/reports/period/formatters/typ/PeriodTypConfig.cpp"

    

    # Shared Components

    "src/reports/shared/utils/config/ConfigUtils.cpp"
    "src/reports/shared/utils/report/ReportDataUtils.cpp"

    "src/reports/shared/utils/format/BoolToString.cpp"
    "src/reports/shared/utils/format/ReportStringUtils.cpp"
    "src/reports/shared/utils/format/TimeFormat.cpp"

    "src/reports/shared/formatters/base/ProjectTreeFormatter.cpp"
    "src/reports/shared/formatters/latex/TexUtils.cpp"

    "src/reports/shared/formatters/markdown/MarkdownUtils.cpp"
    "src/reports/shared/formatters/typst/TypUtils.cpp"
)

# --- Reprocessing Sources ---
set(REPROCESSING_SOURCES
    "src/reprocessing/LogProcessor.cpp"

    # 转换模块
    "src/reprocessing/converter/config/ConverterConfig.cpp"

    "src/reprocessing/converter/facade/IntervalConverter.cpp"

    "src/reprocessing/converter/pipelines/ActivityMapper.cpp"
    "src/reprocessing/converter/pipelines/DayProcessor.cpp"
    "src/reprocessing/converter/pipelines/DayStats.cpp"    
    "src/reprocessing/converter/pipelines/InputParser.cpp"
    "src/reprocessing/converter/pipelines/Output.cpp"

    # 验证模块
    "src/reprocessing/validator/FileValidator.cpp"

    "src/reprocessing/validator/common/ValidatorUtils.cpp"

    # 验证json的封装
    "src/reprocessing/validator/output_json/facade/JsonValidator.cpp"
    # 验证实现
    "src/reprocessing/validator/output_json/pipelines/ActivityRules.cpp"
    "src/reprocessing/validator/output_json/pipelines/DateRules.cpp"
    "src/reprocessing/validator/output_json/pipelines/BusinessRules.cpp"
    "src/reprocessing/validator/output_json/pipelines/TimeRules.cpp"



    "src/reprocessing/validator/source_txt/facade/SourceFacade.cpp"

    "src/reprocessing/validator/source_txt/pipelines/LineRules.cpp"

    "src/reprocessing/validator/source_txt/pipelines/StructureRules.cpp"


)

# --- File Handler Sources ---
set(FILE_HANDLER_SOURCES
    "src/file_handler/ConfigLoader.cpp"
    "src/file_handler/FileController.cpp"
    "src/file_handler/FileUtils.cpp"
)

# --- Action Handler Sources ---
set(ACTION_HANDLER_SOURCES
    "src/action_handler/FileHandler.cpp"
    "src/action_handler/ReportHandler.cpp"
    "src/action_handler/database/DBManager.cpp"
    "src/action_handler/file/PipelineManager.cpp"

    "src/action_handler/reporting/Exporter.cpp"
    "src/action_handler/reporting/ExportUtils.cpp"
    "src/action_handler/reporting/ReportFileManager.cpp"
    "src/action_handler/reporting/ReportGenerator.cpp"
    

)

message(STATUS "Source files collected explicitly.")