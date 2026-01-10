# src/cmake/SourceFileCollection.cmake
# 模块1：源文件收集 (SourceFileCollection.cmake)

# ----------------------------------------------------
# 最佳实践：显式列出所有源文件以确保构建系统的稳定性和可预测性。

# --- Common  ---
set(COMMON_SOURCES
    "src/common/utils/TimeUtils.cpp"
)
# [新增] --- Shared Reports Library ---
# 创建一个静态库，包含所有报告生成器共享的源文件
# 这样可以避免在主程序和多个DLL中重复编译相同的代码
set(REPORTS_SHARED_SOURCES
    "src/common/utils/StringUtils.cpp"
    "src/reports/daily/formatters/base/DayBaseConfig.cpp"
    "src/reports/daily/formatters/statistics/StatFormatter.cpp"
    "src/reports/shared/utils/config/ConfigUtils.cpp"
    "src/reports/shared/utils/report/ReportDataUtils.cpp"
    "src/reports/shared/utils/format/BoolToString.cpp"
    "src/reports/shared/utils/format/ReportStringUtils.cpp"
    "src/reports/shared/utils/format/TimeFormat.cpp"
    "src/reports/shared/formatters/base/ProjectTreeFormatter.cpp"
    "src/reports/shared/formatters/latex/TexUtils.cpp"

    "src/reports/monthly/formatters/base/MonthBaseConfig.cpp"
    "src/reports/period/formatters/base/PeriodBaseConfig.cpp" 
)


# --- 关键修改 1: 将 STATIC 改为 SHARED ---
add_library(reports_shared SHARED ${REPORTS_SHARED_SOURCES})


# --- 关键修改 2: 添加此定义以触发 __declspec(dllexport) ---
target_compile_definitions(reports_shared PRIVATE REPORTS_SHARED_EXPORTS)

# reports_shared 是主程序直接链接的库 ，
# 它不应该被视为“插件”，而应作为“核心组件”。建议将其输出到主程序同级目录
set_target_properties(reports_shared PROPERTIES
    # 将 LIBRARY 和 RUNTIME 的输出路径都指向可执行文件所在的 bin 目录
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# 为这个新的库目标应用通用的编译设置 (头文件路径, 警告等)
# 它会链接 nlohmann_json 等
setup_project_target(reports_shared)

set(CONFIG_VALIDATOR_SOURCES
    # 顶层外观
    "src/config_validator/facade/ConfigFacade.cpp"

    # Reprocessing 领域的 Facade 和 Pipelines
    "src/config_validator/reprocessing/facade/ReprocFacade.cpp"
    "src/config_validator/reprocessing/rules/DurationRule.cpp"
    "src/config_validator/reprocessing/rules/MainRule.cpp"
    "src/config_validator/reprocessing/rules/MappingRule.cpp"

    "src/config_validator/plugins/facade/PluginValidator.cpp"

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
set(CLI_SOURCES
    "src/cli/CliHelp.cpp"
    "src/cli/CommandRegistry.cpp"

    "src/cli/CliController.cpp"
    "src/cli/CliParser.cpp"
    "src/cli/commands/export/Export.cpp"
    

    "src/cli/commands/query/Query.cpp"

    "src/cli/commands/pipeline/Convert.cpp"
    "src/cli/commands/pipeline/Import.cpp"
    "src/cli/commands/pipeline/Run.cpp"
    "src/cli/commands/pipeline/ValidateOutput.cpp"
    "src/cli/commands/pipeline/ValidateSource.cpp"
)
# --- DB Inserter Sources ---
set(DB_BOOTSTRAP_SOURCES
    "src/bootstrap/StartupValidator.cpp"
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
    "src/reports/ReportService.cpp"
    # Export
    "src/reports/services/AllDayReports.cpp"
    "src/reports/services/AllMonthlyReports.cpp"
    "src/reports/services/AllPeriodReports.cpp"
    # Daily Reports
    "src/reports/daily/DayQuerier.cpp"
    # Monthly Reports
    "src/reports/monthly/MonthQuerier.cpp"
    
    # Period Reports
    "src/reports/period/PeriodQuerier.cpp"

    # [核心修改] 所有 period 格式化器的源文件都已被移除
    # 因为它们现在是独立的 DLL 目标
    # "src/reports/period/formatters/md/PeriodMd.cpp"
    # "src/reports/period/formatters/md/PeriodMdConfig.cpp"
    # "src/reports/period/formatters/tex/PeriodTex.cpp"
    # "src/reports/period/formatters/tex/PeriodTexUtils.cpp"
    # "src/reports/period/formatters/tex/PeriodTexConfig.cpp"
    # "src/reports/period/formatters/typ/PeriodTyp.cpp"
    # "src/reports/period/formatters/typ/PeriodTypConfig.cpp"
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
    "src/action_handler/WorkflowHandler.cpp"
    "src/action_handler/ReportHandler.cpp"
    "src/action_handler/database/DBManager.cpp"
    "src/action_handler/file/PipelineManager.cpp"

    # Pipeline steps文件
    "src/action_handler/file/steps/FileCollector.cpp"
    "src/action_handler/file/steps/SourceValidatorStep.cpp"
    "src/action_handler/file/steps/ConverterStep.cpp"
    "src/action_handler/file/steps/OutputValidatorStep.cpp"

    "src/action_handler/reporting/Exporter.cpp"
    "src/action_handler/reporting/ExportUtils.cpp"
    "src/action_handler/reporting/ReportFileManager.cpp"
    "src/action_handler/reporting/ReportGenerator.cpp"
)

message(STATUS "Source files collected explicitly.")