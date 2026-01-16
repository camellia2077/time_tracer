// core/config/ConfigHandler.cpp
#include "core/config/ConfigHandler.hpp"
#include "cli/CommandLineParser.hpp" 
#include "config/Config.hpp"         
#include "common/AnsiColors.hpp"   
#include "io/FileManager.hpp"
#include <filesystem>
#include <iostream>

namespace Core {

    std::optional<AppContext> ConfigHandler::load(int argc, char* argv[]) {
        // 1. 命令行解析
        CommandLineParser parser(argc, argv);
        auto config_opt = parser.parse();
        if (!config_opt) return std::nullopt;

        AppContext context;
        context.config = *config_opt;

        // 2. 路径推导
        std::filesystem::path exe_dir = std::filesystem::path(argv[0]).parent_path();
        // [修改] 删除了 activities_path 的定义
        std::filesystem::path settings_path   = exe_dir / "config" / "activities_config.toml";
        std::filesystem::path mapping_path    = exe_dir / "config" / "mapping_config.toml";

        // 3. [IO操作] 使用 FileManager 读取文件内容
        FileManager file_manager;
        
        // 读取映射配置 (必须)
        auto mapping_content_opt = file_manager.read_file(mapping_path);
        if (!mapping_content_opt) {
            std::cerr << RED_COLOR << "Critical: Failed to read mapping config." << RESET_COLOR << std::endl;
            return std::nullopt;
        }

        // 读取设置配置 (必须)
        auto settings_content_opt = file_manager.read_file(settings_path);
        if (!settings_content_opt) {
            std::cerr << RED_COLOR << "Critical: Failed to read settings config." << RESET_COLOR << std::endl;
            return std::nullopt;
        }

        // [修改] 删除了读取 activities_path (activities_content) 的代码块
        // 之前出错的代码块已完全移除

        // 4. [业务逻辑] 调用 ConfigLoader 解析内容
        ConfigLoader loader;
        
        // [修改] 修正函数调用，只传递两个参数
        auto toml_configs_opt = loader.load_from_content(
            *settings_content_opt, 
            *mapping_content_opt
        );

        if (!toml_configs_opt) {
            std::cerr << RED_COLOR << "程序因配置解析失败而退出。" << RESET_COLOR << std::endl;
            return std::nullopt;
        }

        // 5. 数据组装
        // 这里的 context.all_activities 是生成器随机选择活动的“池子”。
        // 仅使用 mapping_keys (mapped_activities)。
        context.all_activities = toml_configs_opt->mapped_activities;

        // [校验] 确保活动池不为空，否则生成器无法工作
        if (context.all_activities.empty()) {
            std::cerr << RED_COLOR << "Critical: No mapped activities found in mapping_config.toml." << RESET_COLOR << std::endl;
            return std::nullopt;
        }

        context.remarks = toml_configs_opt->remarks;
        context.activity_remarks = toml_configs_opt->activity_remarks;
        context.wake_keywords = toml_configs_opt->wake_keywords;

        return context;
    }

}