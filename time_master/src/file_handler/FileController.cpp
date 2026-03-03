
#include "FileController.hpp"
#include "ConfigLoader.hpp" 
#include "FileUtils.hpp"    
#include <iostream>

// file_handler/FileController.cpp

FileController::FileController(const std::string& exe_path_str) {
    std::cout << "Initializing File Controller..." << std::endl;

    // 使用 ConfigLoader 加载配置，它是一个临时对象
    ConfigLoader loader(exe_path_str);
    
    // 将加载的结果存储在成员变量中
    app_config_ = loader.load_configuration();
    main_config_path_string_ = loader.get_main_config_path();
    
    std::cout << "Configuration loaded successfully." << std::endl;
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