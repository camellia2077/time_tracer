#include "WorkflowHandler.h"
#include "utils/Utils.h"
#include <format>
#include <filesystem>
#include <iostream>

namespace Core {

    // [核心修改] 构造函数注入依赖
    WorkflowHandler::WorkflowHandler(IFileSystem& file_system, ILogGeneratorFactory& generator_factory)
        : file_system_(file_system), generator_factory_(generator_factory) {}

    int WorkflowHandler::run(const AppContext& context, ReportHandler& report_handler) {
        // 1. 使用工厂创建生成器 (不再直接 new LogGenerator)
        auto generator = generator_factory_.create(context);
        
        const std::string master_dir_name = "Date";

        // 2. 使用注入的文件系统接口准备目录
        if (!file_system_.setup_directories(master_dir_name, context.config.start_year, context.config.end_year)) {
            return -1;
        }

        int files_generated = 0;
        auto& reporter = report_handler.get_reporter();

        // 3. 执行生成循环
        for (int year = context.config.start_year; year <= context.config.end_year; ++year) {
            for (int month = 1; month <= 12; ++month) {
                std::string filename = std::format("{}_{:02}.txt", year, month);
                std::filesystem::path full_path = std::filesystem::path(master_dir_name) / std::to_string(year) / filename;

                // 生成
                auto gen_start = std::chrono::high_resolution_clock::now();
                // [修改] 调用接口方法
                std::string month_log = generator->generate_for_month(year, month, Utils::get_days_in_month(year, month));
                reporter.add_generation_time(std::chrono::high_resolution_clock::now() - gen_start);

                // 写入
                auto io_start = std::chrono::high_resolution_clock::now();
                // [修改] 调用接口方法
                if (file_system_.write_log_file(full_path, month_log)) {
                    files_generated++;
                }
                reporter.add_io_time(std::chrono::high_resolution_clock::now() - io_start);
            }
        }

        return files_generated;
    }

}