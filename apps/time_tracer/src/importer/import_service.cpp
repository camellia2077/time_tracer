// importer/import_service.cpp
#include "importer/import_service.hpp"
#include "importer/parser/json_parser.hpp"
#include "importer/parser/memory_parser.hpp"
#include "importer/storage/repository.hpp"
#include <chrono>

ImportService::ImportService(const std::string& db_path) : db_path_(db_path) {}

ImportStats ImportService::import_json_data(const std::vector<std::pair<std::string, std::string>>& inputs) {
    ImportStats stats;
    stats.total_files = inputs.size();
    if (inputs.empty()) return stats;

    auto start_total = std::chrono::high_resolution_clock::now();

    // 1. 解析 (使用纯内存解析器)
    JsonParser parser;
    ParsedData all_data;
    
    for (const auto& item : inputs) {
        const std::string& context_name = item.first;
        const std::string& json_content = item.second;
        try {
            // [核心修改] 传入内容而非路径
            ParsedData single = parser.parse_content(json_content, context_name);
            all_data.days.insert(all_data.days.end(), single.days.begin(), single.days.end());
            all_data.records.insert(all_data.records.end(), single.records.begin(), single.records.end());
        } catch (...) {
            // 记录失败的标识符
            stats.failed_files.push_back(context_name);
        }
    }
    stats.successful_files = stats.total_files - stats.failed_files.size();

    auto end_parsing = std::chrono::high_resolution_clock::now();
    stats.parsing_duration_s = std::chrono::duration<double>(end_parsing - start_total).count();

    // 2. 入库 (保持不变)
    Repository inserter(db_path_);
    if (inserter.is_db_open()) {
        stats.db_open_success = true;
        try {
            inserter.import_data(all_data.days, all_data.records);
            stats.transaction_success = true;
        } catch (const std::exception& e) {
            stats.transaction_success = false;
            stats.error_message = e.what();
        }
    } else {
        stats.db_open_success = false;
        stats.error_message = "Cannot open database: " + db_path_;
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    stats.db_insertion_duration_s = std::chrono::duration<double>(end_total - end_parsing).count();

    return stats;
}

ImportStats ImportService::import_from_memory(const std::map<std::string, std::vector<DailyLog>>& data_map) {
    ImportStats stats;
    for(const auto& p : data_map) stats.total_files += p.second.size();
    stats.successful_files = stats.total_files;
    
    if (data_map.empty()) return stats;

    auto start = std::chrono::high_resolution_clock::now();

    // 1. 转换
    MemoryParser parser;
    ParsedData all_data = parser.parse(data_map);

    auto end_parsing = std::chrono::high_resolution_clock::now();
    stats.parsing_duration_s = std::chrono::duration<double>(end_parsing - start).count();

    // 2. 入库
    Repository inserter(db_path_);
    if (inserter.is_db_open()) {
        stats.db_open_success = true;
        try {
            inserter.import_data(all_data.days, all_data.records);
            stats.transaction_success = true;
        } catch (const std::exception& e) {
            stats.transaction_success = false;
            stats.error_message = e.what();
        }
    } else {
        stats.db_open_success = false;
        stats.error_message = "Cannot open database.";
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    stats.db_insertion_duration_s = std::chrono::duration<double>(end_total - end_parsing).count();

    return stats;
}