#ifndef PROCESSING_H
#define PROCESSING_H

#include <string>
#include <vector> // 建议添加，因为新函数可能会处理路径

/**
 * @brief [交互模式] 模块的公共接口，启动文件处理流程，会提示用户输入。
 * @param db_name 要操作的数据库文件名。
 */
void handle_process_files(const std::string& db_name);

/**
 * @brief [命令行模式] 模块的公共接口，用于处理单个指定的文件或目录。
 * @param db_name 要操作的数据库文件名。
 * @param path 要处理的文件或目录路径。
 */
void handle_process_files(const std::string& db_name, const std::string& path);


#endif // PROCESSING_H