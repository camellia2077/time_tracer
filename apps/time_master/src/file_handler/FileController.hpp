// file_handler/FileController.hpp

#ifndef FILE_CONTROLLER_HPP
#define FILE_CONTROLLER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp" // [修改] 引入新的通用配置头文件

/**
 * @brief 一个外观类，封装了所有底层文件和配置相关的操作。
 * * 这个控制器简化了与文件系统和配置文件的交互。
 * 它在创建时加载应用程序配置，并提供方法来查找文件和访问配置。
 */
class FileController {
public:
    /**
     * @brief 构造 FileController 并加载应用程序配置。
     * @param exe_path_str 应用程序可执行文件的路径 (通常是 argv[0])。
     * @throws std::runtime_error 如果配置加载或验证失败。
     */
    explicit FileController(const std::string& exe_path_str);

    /**
     * @brief 获取已加载的应用程序配置。
     * @return 对 AppConfig 对象的常量引用。
     */
    const AppConfig& get_config() const;

    /**
     * @brief 获取主配置文件的绝对路径。
     * @return 包含路径的字符串。
     */
    std::string get_main_config_path() const;

    /**
     * @brief (新增) 递归地查找指定目录下的日志文件（.txt）。
     * @param root_path 要开始搜索的根目录。
     * @return 一个包含所有找到的 .txt 文件路径的向量。
     */
    std::vector<std::filesystem::path> find_log_files(const std::filesystem::path& root_path) const;

private:
    /**
     * @brief [新增] 在配置加载后执行对预处理相关配置文件的验证。
     * @throws std::runtime_error 如果验证失败。
     */
    void perform_preprocessing_config_validation() const;

    AppConfig app_config_;
    std::string main_config_path_string_;
};

#endif // FILE_CONTROLLER_HPP