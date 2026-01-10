// file_handler/FileController.hpp
#ifndef FILE_CONTROLLER_HPP
#define FILE_CONTROLLER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp"

/**
 * @brief 文件系统外观类。
 * @details 负责加载应用程序的基础配置，并提供底层的文件查找服务。
 * 不包含任何业务规则校验逻辑。
 */
class FileController {
public:
    /**
     * @brief 初始化控制器并加载主配置文件。
     * @param exe_path_str 可执行文件路径。
     */
    explicit FileController(const std::string& exe_path_str);

    const AppConfig& get_config() const;
    std::string get_main_config_path() const;
    
    /**
     * @brief 递归查找指定目录下的日志文件(.txt)。
     */
    std::vector<std::filesystem::path> find_log_files(const std::filesystem::path& root_path) const;

private:
    AppConfig app_config_;
    std::string main_config_path_string_;
};

#endif // FILE_CONTROLLER_HPP