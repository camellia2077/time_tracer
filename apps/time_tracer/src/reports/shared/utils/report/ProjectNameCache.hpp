// reports/shared/utils/report/ProjectNameCache.hpp
#ifndef PROJECT_NAME_CACHE_HPP
#define PROJECT_NAME_CACHE_HPP

#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>

struct ProjectInfo {
    std::string name;
    long long parent_id;
};

class ProjectNameCache {
public:
    static ProjectNameCache& instance() {
        static ProjectNameCache instance;
        return instance;
    }

    // 确保缓存已加载 (懒加载模式)
    void ensure_loaded(sqlite3* db) {
        if (loaded_) return;
        
        const char* sql = "SELECT id, name, parent_id FROM projects";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                long long id = sqlite3_column_int64(stmt, 0);
                const unsigned char* txt = sqlite3_column_text(stmt, 1);
                std::string name = txt ? reinterpret_cast<const char*>(txt) : "";
                long long parent = 0;
                if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
                    parent = sqlite3_column_int64(stmt, 2);
                }
                cache_[id] = {std::move(name), parent};
            }
        } else {
             std::cerr << "Failed to load projects: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
        loaded_ = true;
    }

    // 从 ID 获取完整的路径部分 ["study", "code", "cpp"]
    std::vector<std::string> get_path_parts(long long project_id) const {
        std::vector<std::string> parts;
        long long curr = project_id;
        // 防止死循环 (curr != 0) 且 ID 必须存在
        while (curr != 0 && cache_.count(curr)) {
            const auto& info = cache_.at(curr);
            parts.push_back(info.name);
            curr = info.parent_id;
        }
        std::reverse(parts.begin(), parts.end());
        return parts;
    }

private:
    ProjectNameCache() = default;
    bool loaded_ = false;
    std::unordered_map<long long, ProjectInfo> cache_;
};

#endif // PROJECT_NAME_CACHE_HPP