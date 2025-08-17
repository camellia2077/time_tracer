// --- START OF FILE processing/processing.h ---

#ifndef PROCESSING_H
#define PROCESSING_H

#include <string>

//  db_inserter/DataImporter.h

/**
 * @brief [交互模式] 启动文件处理流程。
 * @details 此函数将组装所需组件，提示用户输入路径，然后执行文件的解析和入库操作。
 * @param db_name 要操作的数据库文件名。
 * @param config_path 指向包含 "initial_top_level_parents" 的主配置文件路径。
 */
void handle_process_files(const std::string& db_name, const std::string& config_path);

/**
 * @brief [命令行模式] 处理单个指定的文件或目录。
 * @details 此函数将组装所需组件，基于给定的路径执行文件的解析和入库操作。
 * @param db_name 要操作的数据库文件名。
 * @param path 要处理的文件或目录路径。
 * @param config_path 指向包含 "initial_top_level_parents" 的主配置文件路径。
 */
void handle_process_files(const std::string& db_name, const std::string& path, const std::string& config_path);

#endif // PROCESSING_H