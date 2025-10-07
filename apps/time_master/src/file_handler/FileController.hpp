// file_handler/FileController.hpp
#ifndef FILE_CONTROLLER_HPP
#define FILE_CONTROLLER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include "common/AppConfig.hpp"

/**
 * @brief 一个外观类，封装了所有底层文件和配置相关的操作。
 */
class FileController {
public:
    /**
     * @brief 构造 FileController 并加载和验证所有应用程序配置。
     * @param exe_path_str 应用程序可执行文件的路径 (通常是 argv[0])。
     * @throws std::runtime_error 如果配置加载或验证失败。
     */
    explicit FileController(const std::string& exe_path_str);

    const AppConfig& get_config() const;
    std::string get_main_config_path() const;
    std::vector<std::filesystem::path> find_log_files(const std::filesystem::path& root_path) const;

private:
    /**
     * @brief 执行对预处理相关配置文件的验证。
     */
    void perform_preprocessing_config_validation() const;

    /**
     * @brief 执行对所有查询报告相关配置文件的验证。
     * @throws std::runtime_error 如果验证失败。
     */
    void perform_query_config_validation() const;

    /**
     * @brief [新增] 执行对所有插件DLL文件的验证。
     * @throws std::runtime_error 如果验证失败。
     */
    void perform_plugin_validation() const;

    AppConfig app_config_;
    std::string main_config_path_string_;
};

#endif // FILE_CONTROLLER_HPP