// core/workflow/WorkflowHandler.hpp
#ifndef WORKFLOW_HANDLER_HPP
#define WORKFLOW_HANDLER_HPP

#include "common/AppContext.hpp"
#include "core/reporting/ReportHandler.hpp"
#include "io/FileSystem.hpp"                 // 依赖抽象
#include "generator/api/LogGeneratorFactory.hpp" // 依赖工厂接口

namespace Core {

    class WorkflowHandler {
    public:
        /**
         * @brief 构造函数，使用依赖注入。
         * @param file_system 文件系统接口的引用
         * @param generator_factory 生成器工厂接口的引用
         */
        WorkflowHandler(FileSystem& file_system, ILogGeneratorFactory& generator_factory);

        int run(const AppContext& context, ReportHandler& report_handler);

    private:
        FileSystem& file_system_;
        ILogGeneratorFactory& generator_factory_;
    };

}

#endif // WORKFLOW_HANDLER_HPP