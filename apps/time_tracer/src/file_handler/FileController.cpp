// file_handler/FileController.cpp
#include "FileController.hpp"
#include "ConfigLoader.hpp"
#include "FileUtils.hpp"
#include <iostream>

FileController::FileController(const std::string& exe_path_str) {
    // 纯粹的配置加载逻辑
    ConfigLoader loader(exe_path_str);
    
    // 1. 读取 config.json
    // 2. 将内容反序列化为 AppConfig 对象
    // 3. 不进行任何业务规则检查 (如 mappings 是否存在)
    app_config_ = loader.load_configuration();
    main_config_path_string_ = loader.get_main_config_path();
}

const AppConfig& FileController::get_config() const {
    return app_config_;
}

std::string FileController::get_main_config_path() const {
    return main_config_path_string_;
}

std::vector<std::filesystem::path> FileController::find_log_files(const std::filesystem::path& root_path) const {
    // 纯粹的文件查找逻辑
    return FileUtils::find_files_by_extension_recursively(root_path, ".txt");
}