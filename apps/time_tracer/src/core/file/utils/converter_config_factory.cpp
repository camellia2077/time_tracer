// core/file/utils/converter_config_factory.cpp
#include "converter_config_factory.hpp"
#include "converter/config/toml_converter_config_loader.hpp"
#include "io/core/file_reader.hpp"
#include "common/ansi_colors.hpp"
#include <toml++/toml.h> 
#include <iostream>

namespace fs = std::filesystem;

static void merge_toml_table(toml::table& target, const toml::table& source) {
    for (const auto& [key, val] : source) {
        if (target.contains(key)) {
            if (target[key].is_table() && val.is_table()) {
                merge_toml_table(*target[key].as_table(), *val.as_table());
            } else {
                target.insert_or_assign(key, val);
            }
        } else {
            target.insert(key, val);
        }
    }
}

static toml::table loadMergedToml(const fs::path& main_config_path) {
    toml::table main_tbl;
    try {
        // 1. 解析主文件
        main_tbl = toml::parse_file(main_config_path.string());
        fs::path config_dir = main_config_path.parent_path();

        // 2. 合并 mappings_config_path
        if (auto path_node = main_tbl["mappings_config_path"].value<std::string>()) {
            fs::path map_path = config_dir / *path_node;
            if (fs::exists(map_path)) {
                toml::table map_tbl = toml::parse_file(map_path.string());
                // 仅合并 text_mappings 部分
                if (map_tbl.contains("text_mappings")) {
                    if (!main_tbl.contains("text_mappings")) {
                        main_tbl.insert("text_mappings", toml::table{});
                    }
                    merge_toml_table(*main_tbl["text_mappings"].as_table(), *map_tbl["text_mappings"].as_table());
                }
            }
        }
        
        // 3. 合并 duration_rules_config_path
        if (auto path_node = main_tbl["duration_rules_config_path"].value<std::string>()) {
            fs::path dur_path = config_dir / *path_node;
            if (fs::exists(dur_path)) {
                toml::table dur_tbl = toml::parse_file(dur_path.string());
                
                // 合并 text_duration_mappings
                if (dur_tbl.contains("text_duration_mappings")) {
                    if (!main_tbl.contains("text_duration_mappings")) {
                        main_tbl.insert("text_duration_mappings", toml::table{});
                    }
                    merge_toml_table(*main_tbl["text_duration_mappings"].as_table(), *dur_tbl["text_duration_mappings"].as_table());
                }

                // 合并 duration_mappings
                if (dur_tbl.contains("duration_mappings")) {
                    if (!main_tbl.contains("duration_mappings")) {
                         main_tbl.insert("duration_mappings", toml::table{});
                    }
                    merge_toml_table(*main_tbl["duration_mappings"].as_table(), *dur_tbl["duration_mappings"].as_table());
                }
            }
        }
    } catch (const toml::parse_error& e) {
        std::cerr << RED_COLOR << "TOML Parse Error: " << e.description() << " (" << e.source().begin << ")" << RESET_COLOR << std::endl;
        throw; 
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Config Merge Error: " << e.what() << RESET_COLOR << std::endl;
        throw; 
    }
    return main_tbl;
}

ConverterConfig ConverterConfigFactory::create(const fs::path& interval_config_path, 
                                             const AppConfig& app_config) {
    // 1. 加载并合并 TOML
    toml::table merged_toml = loadMergedToml(interval_config_path);

    // 2. 使用 TomlLoader 填充配置
    ConverterConfig config;
    TomlConverterConfigLoader loader(merged_toml);
    if (!loader.load(config)) {
        throw std::runtime_error("Failed to populate ConverterConfig from TOML.");
    }

    // 3. 注入运行时参数
    for (const auto& [path_key, path_val] : app_config.pipeline.initial_top_parents) {
        config.initial_top_parents[path_key.string()] = path_val.string();
    }

    return config;
}