// core/Application.cpp
#include "core/Application.hpp"
#include "core/config/ConfigHandler.hpp"
#include "core/workflow/WorkflowHandler.hpp"
#include "core/reporting/ReportHandler.hpp"
#include "utils/Utils.hpp"

// [新增] 引入依赖的头文件
#include "io/FileManager.hpp"
#include "generator/api/LogGeneratorFactory.hpp"

namespace Core {

    int Application::run(int argc, char* argv[]) {
        // 1. 基础环境设置
        Utils::setup_console();

        // 2. 委托 ConfigHandler 加载配置
        ConfigHandler config_handler;
        auto context_opt = config_handler.load(argc, argv);
        if (!context_opt) return 0; // 参数错误或帮助信息，正常退出

        // 3. 初始化 ReportHandler (开始计时)
        ReportHandler report_handler;

        // [修改] 4. 实例化依赖并注入 WorkflowHandler
        // 创建具体的实现对象
        FileManager file_manager;
        LogGeneratorFactory generator_factory;
        
        // 将它们传递给 workflow 的构造函数
        WorkflowHandler workflow(file_manager, generator_factory);
        
        int files_generated = workflow.run(*context_opt, report_handler);

        if (files_generated < 0) return 1; // 运行时错误

        // 5. 委托 ReportHandler 输出结果
        report_handler.finish(context_opt->config, files_generated);

        return 0;
    }

}