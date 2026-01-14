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
    "src/reports/daily/common/DayBaseConfig.cpp"
    "src/reports/daily/formatters/statistics/StatFormatter.cpp"
    "src/reports/shared/utils/config/ConfigUtils.cpp"
    "src/reports/shared/utils/tree/ProjectTreeBuilder.cpp"

    "src/reports/shared/utils/format/BoolToString.cpp"
    "src/reports/shared/utils/format/ReportStringUtils.cpp"
    "src/reports/shared/utils/format/TimeFormat.cpp"
    "src/reports/shared/formatters/base/ProjectTreeFormatter.cpp"

    "src/reports/shared/config/TexStyleConfig.cpp"
    "src/reports/shared/config/TypstStyleConfig.cpp"

    "src/reports/shared/formatters/markdown/MarkdownFormatter.cpp" 
    "src/reports/shared/formatters/typst/TypUtils.cpp"
    "src/reports/shared/formatters/latex/TexUtils.cpp"
    "src/reports/shared/formatters/latex/TexCommonUtils.cpp"

    "src/reports/monthly/common/MonthBaseConfig.cpp"
    "src/reports/period/common/PeriodBaseConfig.cpp" 
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



# --- Time Master CLI Sources ---
set(CLI_SOURCES
    "src/cli/CliHelp.cpp"
    "src/cli/CommandRegistry.cpp"
    "src/cli/InputPrompter.cpp"
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
set(BOOTSTRAP_SOURCES
    "src/bootstrap/StartupValidator.cpp"
)

# --- Importer Sources (Database Ingestion Module) ---
set(IMPORTER_SOURCES
    # Top-level Facade & Service
    "src/importer/DataImporter.cpp"
    "src/importer/ImportService.cpp"

    # Parser Layer (Parsing logic)
    "src/importer/parser/JsonParser.cpp"
    "src/importer/parser/MemoryParser.cpp"
    "src/importer/parser/details/DayParser.cpp"
    "src/importer/parser/details/ActivityParser.cpp"

    # Storage Layer (Database persistence)
    "src/importer/storage/Repository.cpp"
    "src/importer/storage/sqlite/Writer.cpp"
    "src/importer/storage/sqlite/ProjectResolver.cpp"
    "src/importer/storage/sqlite/Connection.cpp"
    "src/importer/storage/sqlite/Statement.cpp"
)

set(REPORTS_SOURCES
    # Root
    "src/reports/ReportService.cpp"
    # Export
    "src/reports/services/DailyReportService.cpp"
    "src/reports/services/MonthlyReportService.cpp"
    "src/reports/services/PeriodReportService.cpp"
    # Daily Reports
    "src/reports/daily/queriers/DayQuerier.cpp"
    "src/reports/daily/queriers/DayQuerier.cpp"
    "src/reports/daily/queriers/BatchDayDataFetcher.cpp"
    # Monthly Reports
    "src/reports/monthly/queriers/MonthQuerier.cpp"
    "src/reports/monthly/queriers/BatchMonthDataFetcher.cpp"
    
    
    # Period Reports
    "src/reports/period/queriers/PeriodQuerier.cpp"
    "src/reports/period/queriers/BatchPeriodDataFetcher.cpp"

)

# --- converter Sources ---
set(CONVERTER_SOURCES
    "src/converter/LogProcessor.cpp"

    # --- 配置模块 (Config) [Moved] ---
    "src/converter/config/ConverterConfig.cpp"
    "src/converter/config/JsonConverterConfigLoader.cpp"

    # --- 转换模块 (Convert) ---
    # Facade
    "src/converter/convert/facade/ConverterService.cpp"
    
    # Core (原 pipelines 中的算法部分) [New Path]
    "src/converter/convert/core/ActivityMapper.cpp"
    "src/converter/convert/core/DayProcessor.cpp"
    "src/converter/convert/core/DayStats.cpp"

    # IO (原 pipelines 中的解析与输出部分) [New Path]
    "src/converter/convert/io/TextParser.cpp"
    "src/converter/convert/io/JsonWriter.cpp"

    # --- 验证模块 (Validator) ---
    "src/converter/validator/FileValidator.cpp"
    "src/converter/validator/common/ValidatorUtils.cpp"

    # Json 验证
    "src/converter/validator/output_json/facade/JsonValidator.cpp"
    "src/converter/validator/output_json/rules/ActivityRules.cpp"
    "src/converter/validator/output_json/rules/DateRules.cpp"

    # Source 文本验证
    "src/converter/validator/source_txt/facade/TextValidator.cpp"
    "src/converter/validator/source_txt/rules/LineRules.cpp"
    "src/converter/validator/source_txt/rules/StructureRules.cpp"
)

# --- File Handler Sources ---
set(IO_SOURCES
    "src/io/FileController.cpp"
    "src/io/core/FileReader.cpp"
    "src/io/core/FileWriter.cpp"
    "src/io/core/FileSystemHelper.cpp"
    "src/io/utils/FileUtils.cpp"
)

set(CONFIG_SOURCES
    "src/config/ConfigLoader.cpp"

    "src/config/internal/ConfigParserUtils.cpp"
    
    "src/config/loader/JsonLoaderUtils.cpp"
    "src/config/loader/ReportConfigLoader.cpp"


    # 顶层外观
    "src/config/validator/facade/ConfigFacade.cpp"

    # converter 领域的 Facade 和 Pipelines
    "src/config/validator/converter/facade/ConverterFacade.cpp"
    "src/config/validator/converter/rules/DurationRule.cpp"
    "src/config/validator/converter/rules/MainRule.cpp"
    "src/config/validator/converter/rules/MappingRule.cpp"

    "src/config/validator/plugins/facade/PluginValidator.cpp"

    # reports 领域的 Facade
    "src/config/validator/reports/facade/QueryFacade.cpp"

    # reports 领域的 Strategies (核心)
    "src/config/validator/reports/strategies/BaseStrategy.cpp"
    "src/config/validator/reports/strategies/StrategyFactory.cpp"

    # reports 领域的 Strategies (具体实现)
    "src/config/validator/reports/strategies/daily/DailyMd.cpp"
    "src/config/validator/reports/strategies/daily/DailyTex.cpp"
    "src/config/validator/reports/strategies/daily/DailyTyp.cpp"
    "src/config/validator/reports/strategies/monthly/Monthly.cpp"
    "src/config/validator/reports/strategies/periodic/Periodic.cpp"
)



# --- Core Sources ---
set(CORE_SOURCES
    "src/core/WorkflowHandler.cpp"
    "src/core/ReportHandler.cpp"
    "src/core/database/DBManager.cpp"
    "src/core/file/PipelineManager.cpp"

    # Pipeline steps文件
    "src/core/file/steps/FileCollector.cpp"
    "src/core/file/steps/SourceValidatorStep.cpp"
    "src/core/file/steps/ConverterStep.cpp"
    "src/core/file/steps/OutputValidatorStep.cpp"

    "src/core/file/utils/ConverterConfigFactory.cpp"
    "src/core/file/utils/ProcessedDataWriter.cpp"

    "src/core/reporting/Exporter.cpp"
    "src/core/reporting/ExportUtils.cpp"
    "src/core/reporting/ReportFileManager.cpp"
    "src/core/reporting/ReportGenerator.cpp"
)

message(STATUS "Source files collected explicitly.")