#include "WorkflowHandler.h"
#include "utils/Utils.h"
#include <format>
#include <filesystem>
#include <iostream>
#include <vector>
#include <future>
#include <thread>

namespace Core {

    WorkflowHandler::WorkflowHandler(IFileSystem& file_system, ILogGeneratorFactory& generator_factory)
        : file_system_(file_system), generator_factory_(generator_factory) {}

    int WorkflowHandler::run(const AppContext& context, ReportHandler& report_handler) {
        const std::string master_dir_name = "Date";

        // IO: 创建目录 (串行操作，避免竞争)
        if (!file_system_.setup_directories(master_dir_name, context.config.start_year, context.config.end_year)) {
            return -1;
        }

        auto& reporter = report_handler.get_reporter();
        
        // 任务列表：使用 std::future 获取每个并行任务的结果（生成文件数）
        std::vector<std::future<int>> tasks;

        // [逻辑] 并行粒度：以“年”为单位。
        for (int year = context.config.start_year; year <= context.config.end_year; ++year) {
            
            // [修复] 在捕获列表中显式添加 'this'
            // C++20 之前 [=] 会隐式捕获 this，C++20 要求显式写出 [=, this, ...] 以表明意图
            tasks.push_back(std::async(std::launch::async, [=, this, &context, &reporter, &master_dir_name]() {
                
                // [线程局部] 1. 创建独立的生成器实例 (Thread Safety)
                // 这里使用了成员变量 generator_factory_，所以需要捕获 this
                auto generator = generator_factory_.create(context);
                
                // [线程局部] 2. 预分配复用缓冲区 (Memory Reuse)
                std::string buffer;
                buffer.reserve(1024 * 1024); // 预估 1MB，足够容纳单月数据

                int local_files_generated = 0;

                for (int month = 1; month <= 12; ++month) {
                    std::string filename = std::format("{}_{:02}.txt", year, month);
                    std::filesystem::path full_path = std::filesystem::path(master_dir_name) / std::to_string(year) / filename;

                    // 计算
                    auto gen_start = std::chrono::high_resolution_clock::now();
                    // 传入 buffer 进行复用
                    generator->generate_for_month(year, month, Utils::get_days_in_month(year, month), buffer);
                    reporter.add_generation_time(std::chrono::high_resolution_clock::now() - gen_start);

                    // IO
                    auto io_start = std::chrono::high_resolution_clock::now();
                    // 写入 (FileManager 应当是无状态或线程安全的)
                    // 这里使用了成员变量 file_system_，所以需要捕获 this
                    if (file_system_.write_log_file(full_path, buffer)) {
                        local_files_generated++;
                    }
                    reporter.add_io_time(std::chrono::high_resolution_clock::now() - io_start);
                }

                return local_files_generated;
            }));
        }

        // 等待所有任务完成并汇总结果
        int total_files_generated = 0;
        for (auto& task : tasks) {
            total_files_generated += task.get(); // get() 会阻塞直到任务完成
        }

        return total_files_generated;
    }

}