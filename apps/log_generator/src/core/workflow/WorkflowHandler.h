#ifndef CORE_WORKFLOW_WORKFLOWHANDLER_H
#define CORE_WORKFLOW_WORKFLOWHANDLER_H

#include "common/AppContext.h"
#include "core/reporting/ReportHandler.h"
#include "io/IFileSystem.h"                 // 依赖抽象
#include "generator/facade/LogGeneratorFactory.h" // 依赖工厂接口

namespace Core {

    class WorkflowHandler {
    public:
        /**
         * @brief 构造函数，使用依赖注入。
         * @param file_system 文件系统接口的引用
         * @param generator_factory 生成器工厂接口的引用
         */
        WorkflowHandler(IFileSystem& file_system, ILogGeneratorFactory& generator_factory);

        int run(const AppContext& context, ReportHandler& report_handler);

    private:
        IFileSystem& file_system_;
        ILogGeneratorFactory& generator_factory_;
    };

}

#endif // CORE_WORKFLOW_WORKFLOWHANDLER_H