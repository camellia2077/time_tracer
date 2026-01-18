// importer/data_importer.hpp

#ifndef IMPORTER_DATA_IMPORTER_HPP_
#define IMPORTER_DATA_IMPORTER_HPP_

#include <string>
#include <vector>
#include <map>
#include <utility> // for std::pair

struct DailyLog; 

// ==========================================
// Facade Interface (UI/CLI Layer Entry Points)
// ==========================================

/**
 * @brief [修改] 处理 JSON 内容字符串的导入。
 * @param db_name 数据库路径。
 * @param inputs 输入数据列表，<identifier, content>。
 */
void handle_import_json_content(const std::string& db_name, const std::vector<std::pair<std::string, std::string>>& inputs);

// 保持不变
void handle_process_memory_data(const std::string& db_name, const std::map<std::string, std::vector<DailyLog>>& data_map);

#endif // IMPORTER_DATA_IMPORTER_HPP_