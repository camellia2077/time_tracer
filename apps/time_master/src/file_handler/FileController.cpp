// file_handler/FileController.cpp
#include "FileController.hpp"
#include "ConfigLoader.hpp"
#include "FileUtils.hpp"
#include "config_validator/facade/ConfigValidator.hpp" // [修改] 更新为facade的路径
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

// 辅助函数，用于从此模块内部加载JSON文件
static bool load_json_from_file(const std::string& path, nlohmann::json& out_json) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "错误: 无法打开文件: " << path << std::endl;
        return false;
    }
    try {
        ifs >> out_json;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "错误: 解析 " << path << " 的JSON时失败: " << e.what() << std::endl;
        return false;
    }
}


FileController::FileController(const std::string& exe_path_str) {
    std::cout << "正在初始化文件控制器..." << std::endl;

    ConfigLoader loader(exe_path_str);
    app_config_ = loader.load_configuration();
    main_config_path_string_ = loader.get_main_config_path();

    std::cout << "主应用配置加载成功。" << std::endl;

    // 依次执行两种类型的配置验证
    perform_preprocessing_config_validation();
    perform_query_config_validation(); // [新增] 调用查询配置验证
}

const AppConfig& FileController::get_config() const {
    return app_config_;
}

std::string FileController::get_main_config_path() const {
    return main_config_path_string_;
}

std::vector<std::filesystem::path> FileController::find_log_files(const std::filesystem::path& root_path) const {
    return FileUtils::find_files_by_extension_recursively(root_path, ".txt");
}

void FileController::perform_preprocessing_config_validation() const {
    std::cout << "正在验证预处理配置文件..." << std::endl;
    
    nlohmann::json main_json, mappings_json, duration_rules_json;

    if (!load_json_from_file(app_config_.interval_processor_config_path, main_json)) {
        throw std::runtime_error("无法加载主预处理配置文件，操作中止。");
    }

    std::filesystem::path base_dir = std::filesystem::path(app_config_.interval_processor_config_path).parent_path();
    std::string mappings_file = main_json.value("mappings_config_path", "");
    std::string duration_file = main_json.value("duration_rules_config_path", "");

    if (mappings_file.empty() || duration_file.empty()) {
        throw std::runtime_error("主预处理配置文件中缺少 'mappings_config_path' 或 'duration_rules_config_path'。");
    }

    if (!load_json_from_file((base_dir / mappings_file).string(), mappings_json) ||
        !load_json_from_file((base_dir / duration_file).string(), duration_rules_json)) {
        throw std::runtime_error("无法加载映射或时长规则配置文件，操作中止。");
    }

    ConfigValidator validator;
    // [修改] 调用更新后的方法
    if (!validator.validate_preprocessing_configs(main_json, mappings_json, duration_rules_json)) {
        throw std::runtime_error("预处理配置文件验证失败。请检查上面的错误信息。");
    }
}

// [新增] 实现查询配置的验证逻辑
void FileController::perform_query_config_validation() const {
    std::cout << "正在验证查询配置文件..." << std::endl;

    ConfigValidator validator;
    std::vector<std::pair<std::string, nlohmann::json>> query_jsons;

    const std::vector<std::string> query_config_paths = {
        app_config_.day_tex_config_path, app_config_.month_tex_config_path, app_config_.period_tex_config_path,
        app_config_.day_md_config_path, app_config_.month_md_config_path, app_config_.period_md_config_path,
        app_config_.day_typ_config_path, app_config_.month_typ_config_path, app_config_.period_typ_config_path
    };

    for (const auto& path_str : query_config_paths) {
        if (path_str.empty()) continue; // 如果路径为空则跳过

        std::filesystem::path p(path_str);
        nlohmann::json q_json;
        if (load_json_from_file(p.string(), q_json)) {
            query_jsons.push_back({p.filename().string(), q_json});
        } else {
            // 如果任何一个文件加载失败，则立即终止
            throw std::runtime_error("无法加载查询配置文件 '" + p.filename().string() + "'，操作中止。");
        }
    }

    if (!validator.validate_query_configs(query_jsons)) {
        throw std::runtime_error("查询配置文件验证失败。请检查上面的错误信息。");
    }
}