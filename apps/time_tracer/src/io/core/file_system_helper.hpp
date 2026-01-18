// io/core/file_system_helper.hpp
#ifndef IO_CORE_FILE_SYSTEM_HELPER_HPP_
#define IO_CORE_FILE_SYSTEM_HELPER_HPP_

#include <filesystem>

/**
 * @brief 文件系统操作的封装助手类。
 * @details 提供了对 std::filesystem 常见操作的包装，
 * 旨在统一异常处理并解耦上层业务逻辑与底层文件系统 API。
 */
class FileSystemHelper {
public:
    /**
     * @brief 检查路径是否存在。
     */
    static bool exists(const std::filesystem::path& path);

    /**
     * @brief 检查路径是否为普通文件。
     */
    static bool is_regular_file(const std::filesystem::path& path);

    /**
     * @brief 检查路径是否为目录。
     */
    static bool is_directory(const std::filesystem::path& path);

    /**
     * @brief 递归创建目录。
     * @details 类似于 mkdir -p。如果目录已存在，则不执行任何操作。
     * @param path 要创建的目录路径。
     * @throws std::runtime_error 如果创建失败。
     */
    static void create_directories(const std::filesystem::path& path);
};

#endif // IO_CORE_FILE_SYSTEM_HELPER_HPP_