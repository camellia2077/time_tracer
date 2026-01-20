// reports/data/cache/project_name_cache.hpp
#ifndef REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_HPP_
#define REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_HPP_

#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iostream>
#include "reports/data/interfaces/i_project_info_provider.hpp"

struct ProjectInfo {
    std::string name;
    long long parent_id;
};

// [修改] 继承 IProjectInfoProvider
class ProjectNameCache : public IProjectInfoProvider {
public:
    static ProjectNameCache& instance() {
        static ProjectNameCache instance;
        return instance;
    }

    // [修改] 添加 override 关键字
    void ensure_loaded(sqlite3* db) override {
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

    // [修改] 添加 override 关键字
    std::vector<std::string> get_path_parts(long long project_id) const override {
        std::vector<std::string> parts;
        long long curr = project_id;
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

#endif // REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_HPP_