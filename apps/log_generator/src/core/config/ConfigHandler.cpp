// core/config/ConfigHandler.cpp
#include "core/config/ConfigHandler.h"
#include "cli/CommandLineParser.h" 
#include "config/Config.h"         
#include "common/AnsiColors.hpp"   
#include "io/FileManager.h" // [引用] 引入IO模块
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
        std::filesystem::path activities_path = exe_dir / "config" / "activities_config.toml"; 
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

        // 读取活动配置 (可选/容错)
        // 注意：在本例中 activities_path 和 settings_path 是同一个文件，
        // 但为了逻辑独立性，我们分别处理。如果文件不存在，传空字符串。
        auto activities_content_opt = file_manager.read_file(activities_path);
        std::string activities_content = activities_content_opt.value_or("");

        // 4. [业务逻辑] 调用 ConfigLoader 解析内容
        ConfigLoader loader;
        auto toml_configs_opt = loader.load_from_content(
            activities_content, 
            *settings_content_opt, 
            *mapping_content_opt
        );

        if (!toml_configs_opt) {
            std::cerr << RED_COLOR << "程序因配置解析失败而退出。" << RESET_COLOR << std::endl;
            return std::nullopt;
        }

        // 5. 数据组装
        context.all_activities = toml_configs_opt->activities;
        context.all_activities.insert(context.all_activities.end(), 
                                      toml_configs_opt->mapped_activities.begin(), 
                                      toml_configs_opt->mapped_activities.end());
        context.remarks = toml_configs_opt->remarks;
        context.activity_remarks = toml_configs_opt->activity_remarks;
        context.wake_keywords = toml_configs_opt->wake_keywords;

        return context;
    }

}