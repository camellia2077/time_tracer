// core/reporting/ReportHandler.h
#ifndef CORE_REPORTING_REPORTHANDLER_H
#define CORE_REPORTING_REPORTHANDLER_H

#include "utils/PerformanceReporter.h"
#include <chrono>

namespace Core {

    class ReportHandler {
    public:
        ReportHandler();
        
        // 获取底层的 Reporter 以便其他模块记录数据
        PerformanceReporter& get_reporter();

        // 结束计时并打印报告
        void finish(const Config& config, int files_generated);

    private:
        PerformanceReporter reporter_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    };

}

#endif // CORE_REPORTING_REPORTHANDLER_H