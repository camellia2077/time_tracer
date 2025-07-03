#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <filesystem>
#include "LogProcessor.h" // For the AppConfig structure
#include "common_utils.h" // For color macros

class FileHandler {
public:
    /**
     * @brief Constructs a FileHandler.
     * @param exe_path_str The path to the executable, typically from argv[0].
     */
    FileHandler(const std::string& exe_path_str);

    /**
     * @brief Loads all application configurations from files.
     * * It finds and parses the main 'config.json' and then resolves the paths
     * to other configuration files specified within it.
     * * @return A populated AppConfig object.
     * @throws std::runtime_error on file-not-found, parsing, or permission errors.
     */
    AppConfig load_configuration();

    /**
     * @brief Gets the absolute path to the main config.json file.
     * @return A string containing the full path.
     */
    std::string get_main_config_path() const;

    /**
     * @brief (新增) 递归地查找指定目录及其所有子目录下的 .txt 文件。
     * @param root_path 要开始搜索的根目录。
     * @return 一个包含所有找到的 .txt 文件路径的向量(std::vector)，并已按字母顺序排序。
     */
    static std::vector<std::filesystem::path> find_text_files_recursively(const std::filesystem::path& root_path);

private:
    std::filesystem::path exe_path;
    std::filesystem::path config_dir_path;
    std::filesystem::path main_config_path;

    // Constants defining key configuration file and directory names
    const std::string CONFIG_FILE_NAME = "config.json";
    const std::string CONFIG_DIR_NAME = "config"; 
};

#endif // FILE_HANDLER_H