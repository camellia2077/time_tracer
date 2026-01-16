// core/workflow/WorkflowHandler.cpp
#include "WorkflowHandler.hpp"
#include "utils/Utils.hpp"
#include <format>
#include <filesystem>
#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <algorithm> // for std::remove_if

namespace Core {

    WorkflowHandler::WorkflowHandler(FileSystem& file_system, ILogGeneratorFactory& generator_factory)
        : file_system_(file_system), generator_factory_(generator_factory) {}

    int WorkflowHandler::run(const AppContext& context, ReportHandler& report_handler) {
        const std::string master_dir_name = "Date";

        // IO: 创建目录 (串行操作，避免竞争)
        if (!file_system_.setup_directories(master_dir_name, context.config.start_year, context.config.end_year)) {
            return -1;
        }

        auto& reporter = report_handler.get_reporter();
        
        // [优化] 1. 确定并发度 (使用硬件核心数，兜底为 4)
        unsigned int max_concurrent = std::thread::hardware_concurrency();
        if (max_concurrent == 0) max_concurrent = 4;
        
        // 打印提示，让用户知道当前的并行力度
        std::cout << "Starting generation with " << max_concurrent << " concurrent threads...\n";

        // 任务列表：存储正在运行的 future
        std::vector<std::future<int>> active_tasks;
        int total_files_generated = 0;

        for (int year = context.config.start_year; year <= context.config.end_year; ++year) {
            
            // [优化] 2. 检查任务队列，如果满了就清理已完成的任务
            // 使用 remove_if 惯用语清理已经 ready 的任务
            auto it = std::remove_if(active_tasks.begin(), active_tasks.end(), 
                [&total_files_generated](std::future<int>& f) {
                    if (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                        total_files_generated += f.get(); // 获取结果并累加
                        return true; // 标记为可删除
                    }
                    return false;
                });
            active_tasks.erase(it, active_tasks.end());

            // [优化] 3. 如果清理后仍然达到上限，强制等待最早的一个任务完成 (Backpressure)
            if (active_tasks.size() >= max_concurrent) {
                total_files_generated += active_tasks.front().get();
                active_tasks.erase(active_tasks.begin());
            }

            // [逻辑] 启动新任务 (代码逻辑保持不变，只是被放入了受控循环中)
            active_tasks.push_back(std::async(std::launch::async, [=, this, &context, &reporter, &master_dir_name]() {
                
                auto generator = generator_factory_.create(context);
                
                std::string buffer;
                buffer.reserve(1024 * 1024); // 预估 1MB

                int local_files_generated = 0;

                for (int month = 1; month <= 12; ++month) {
                    std::string filename = std::format("{}_{:02}.txt", year, month);
                    std::filesystem::path full_path = std::filesystem::path(master_dir_name) / std::to_string(year) / filename;

                    // 计算
                    auto gen_start = std::chrono::high_resolution_clock::now();
                    generator->generate_for_month(year, month, Utils::get_days_in_month(year, month), buffer);
                    reporter.add_generation_time(std::chrono::high_resolution_clock::now() - gen_start);

                    // IO
                    auto io_start = std::chrono::high_resolution_clock::now();
                    if (file_system_.write_log_file(full_path, buffer)) {
                        local_files_generated++;
                    }
                    reporter.add_io_time(std::chrono::high_resolution_clock::now() - io_start);
                }

                return local_files_generated;
            }));
        }

        // [优化] 4. 循环结束后，等待所有剩余任务完成
        for (auto& task : active_tasks) {
            total_files_generated += task.get();
        }

        return total_files_generated;
    }

}