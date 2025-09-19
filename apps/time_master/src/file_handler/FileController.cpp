// file_handler/FileController.cpp
#include "FileController.hpp"
#include "ConfigLoader.hpp"
#include "FileUtils.hpp"
#include "config_validator/ConfigValidator.hpp" // [新增] 引入配置验证器
#include <nlohmann/json.hpp>                   // [新增] 引入 nlohmann/json
#include <iostream>
#include <fstream>      // [新增] 引入 fstream
#include <stdexcept>    // [新增] 引入 stdexcept


// [新增] 辅助函数，用于从此模块内部加载JSON文件
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

    // 使用 ConfigLoader 加载配置，它是一个临时对象
    ConfigLoader loader(exe_path_str);

    // 将加载的结果存储在成员变量中
    app_config_ = loader.load_configuration();
    main_config_path_string_ = loader.get_main_config_path();

    std::cout << "主应用配置加载成功。" << std::endl;

    // [新增] 调用新的验证方法
    perform_preprocessing_config_validation();
}

const AppConfig& FileController::get_config() const {
    return app_config_;
}

std::string FileController::get_main_config_path() const {
    return main_config_path_string_;
}

std::vector<std::filesystem::path> FileController::find_log_files(const std::filesystem::path& root_path) const {
    // 封装对 FileUtils 的调用，简化上层代码
    return FileUtils::find_files_by_extension_recursively(root_path, ".txt");
}

// [新增] 实现预处理配置的验证逻辑
void FileController::perform_preprocessing_config_validation() const {
    std::cout << "正在验证预处理配置文件..." << std::endl;
    
    nlohmann::json main_json, mappings_json, duration_rules_json;

    // 1. 加载主预处理配置文件 (interval_processor_config.json)
    if (!load_json_from_file(app_config_.interval_processor_config_path, main_json)) {
        throw std::runtime_error("无法加载主预处理配置文件，操作中止。");
    }

    // 2. 从主配置中获取其他配置文件的路径
    std::filesystem::path base_dir = std::filesystem::path(app_config_.interval_processor_config_path).parent_path();
    std::string mappings_file = main_json.value("mappings_config_path", "");
    std::string duration_file = main_json.value("duration_rules_config_path", "");

    if (mappings_file.empty() || duration_file.empty()) {
        throw std::runtime_error("主预处理配置文件中缺少 'mappings_config_path' 或 'duration_rules_config_path'。");
    }

    // 3. 加载子配置文件
    if (!load_json_from_file((base_dir / mappings_file).string(), mappings_json) ||
        !load_json_from_file((base_dir / duration_file).string(), duration_rules_json)) {
        throw std::runtime_error("无法加载映射或时长规则配置文件，操作中止。");
    }

    // 4. 调用验证器
    ConfigValidator validator;
    if (!validator.validate(main_json, mappings_json, duration_rules_json)) {
        // 验证器内部会打印详细错误，这里我们抛出异常来终止程序
        throw std::runtime_error("预处理配置文件验证失败。请检查上面的错误信息。");
    }

    std::cout << "预处理配置文件验证通过。" << std::endl;
}