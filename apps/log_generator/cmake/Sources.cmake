
# 负责管理源文件列表
# --- Source Files Definition ---

set(SOURCES
    # 入口
    src/main.cpp
    
    # Core 核心模块
    src/core/Application.cpp
    src/core/config/ConfigHandler.cpp      
    src/core/workflow/WorkflowHandler.cpp 
    src/core/reporting/ReportHandler.cpp
    
    # CLI 命令行模块
    src/cli/CommandLineParser.cpp
    src/cli/HelpPrinter.cpp       
    
    # Config 配置模块 (旧有的类)
    src/config/Config.cpp
    src/config/ConfigFacade.cpp 
    
    # FileIO 文件模块
    src/io/FileManager.cpp
    
    # Generator 生成器模块
    src/generator/components/RemarkGenerator.cpp
    src/generator/components/EventGenerator.cpp
    src/generator/components/DayGenerator.cpp

    # Impl (核心实现)
    src/generator/impl/LogGenerator.cpp

    # Strategies (策略/算法)
    src/generator/strategies/SleepScheduler.cpp
    
    # Utils 工具模块
    src/utils/PerformanceReporter.cpp
    src/utils/Utils.cpp
)