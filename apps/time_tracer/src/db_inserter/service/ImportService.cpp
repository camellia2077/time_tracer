// db_inserter/service/ImportService.cpp
#include "db_inserter/service/ImportService.hpp"
#include "db_inserter/parser/facade/JsonParserFacade.hpp"
#include "db_inserter/parser/facade/MemoryParserFacade.hpp"
#include "db_inserter/inserter/facade/DatabaseInserter.hpp"
#include <chrono>

ImportService::ImportService(const std::string& db_path) : db_path_(db_path) {}

ImportStats ImportService::import_from_files(const std::vector<std::string>& files) {
    ImportStats stats;
    stats.total_files = files.size();
    if (files.empty()) return stats;

    auto start_total = std::chrono::high_resolution_clock::now();

    // 1. 解析
    JsonParserFacade parser;
    ParsedData all_data;
    
    for (const std::string& fname : files) {
        try {
            ParsedData single = parser.parse_file(fname);
            all_data.days.insert(all_data.days.end(), single.days.begin(), single.days.end());
            all_data.records.insert(all_data.records.end(), single.records.begin(), single.records.end());
        } catch (...) {
            stats.failed_files.push_back(fname);
        }
    }
    stats.successful_files = stats.total_files - stats.failed_files.size();

    auto end_parsing = std::chrono::high_resolution_clock::now();
    stats.parsing_duration_s = std::chrono::duration<double>(end_parsing - start_total).count();

    // 2. 入库
    DatabaseInserter inserter(db_path_);
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

ImportStats ImportService::import_from_memory(const std::map<std::string, std::vector<InputData>>& data_map) {
    ImportStats stats;
    for(const auto& p : data_map) stats.total_files += p.second.size();
    stats.successful_files = stats.total_files;
    
    if (data_map.empty()) return stats;

    auto start = std::chrono::high_resolution_clock::now();

    // 1. 转换
    MemoryParserFacade parser;
    ParsedData all_data = parser.parse(data_map);

    auto end_parsing = std::chrono::high_resolution_clock::now();
    stats.parsing_duration_s = std::chrono::duration<double>(end_parsing - start).count();

    // 2. 入库
    DatabaseInserter inserter(db_path_);
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