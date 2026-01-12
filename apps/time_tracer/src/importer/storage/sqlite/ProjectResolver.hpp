// importer/storage/sqlite/ProjectResolver.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sqlite3.h>

struct ImportProjectNode; // 前置声明

class ProjectResolver {
public:
    explicit ProjectResolver(sqlite3* db, sqlite3_stmt* stmt_insert_project);
    ~ProjectResolver(); // 必须有析构函数

    // 批量预处理 (替代 Writer::preprocess_projects)
    void preload_and_resolve(const std::vector<std::string>& project_paths);
    
    // 获取 ID (替代 full_path_id_cache_ 的直接访问)
    long long get_id(const std::string& project_path) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_insert_project_;

    std::unique_ptr<ImportProjectNode> root_;
    std::unordered_map<std::string, long long> cache_;

    void load_from_db();
    long long ensure_path(const std::string& path);
};