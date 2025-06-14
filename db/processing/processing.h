#ifndef PROCESSING_H
#define PROCESSING_H

#include <string> // 因为函数参数需要 std::string

/**
 * @brief 模块的公共接口，用于启动整个文件处理和导入流程。
 * @param db_name 要操作的数据库文件名。
 */
void handle_process_files(const std::string& db_name);

#endif // PROCESSING_H