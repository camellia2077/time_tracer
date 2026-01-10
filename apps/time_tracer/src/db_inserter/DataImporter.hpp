// db_inserter/DataImporter.hpp
#ifndef DATA_IMPORTER_HPP
#define DATA_IMPORTER_HPP

#include <string>
#include <vector>
#include <map>

// 前置声明，避免暴露内部模型细节
struct InputData; 

// ==========================================
// Facade Interface (UI/CLI Layer Entry Points)
// ==========================================

void handle_process_files(const std::string& db_name, const std::vector<std::string>& paths);

void handle_process_files(const std::string& db_name, const std::string& path);

void handle_process_memory_data(const std::string& db_name, const std::map<std::string, std::vector<InputData>>& data_map);

#endif // DATA_IMPORTER_HPP