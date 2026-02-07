// infrastructure/reports/data/cache/project_name_cache.hpp
#ifndef REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_
#define REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_

#include <sqlite3.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/reports/interfaces/i_project_info_provider.hpp"

struct ProjectInfo {
  std::string name;
  long long parent_id;
};

// [修改] 继承 IProjectInfoProvider
class ProjectNameCache : public IProjectInfoProvider {
 public:
  static ProjectNameCache& Instance() {
    static ProjectNameCache instance;
    return instance;
  }

  // [修改] 添加 override 关键字
  void EnsureLoaded(sqlite3* sqlite_db) override {
    if (loaded_) return;

    // Use sqlite_db instead of db
    const char* sql = "SELECT id, name, parent_id FROM projects";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long project_id = sqlite3_column_int64(stmt, 0);
        const unsigned char* txt = sqlite3_column_text(stmt, 1);
        std::string name =
            (txt != nullptr) ? reinterpret_cast<const char*>(txt) : "";
        long long parent = 0;
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
          parent = sqlite3_column_int64(stmt, 2);
        }
        cache_[project_id] = {.name = std::move(name), .parent_id = parent};
      }
    } else {
      std::cerr << "Failed to load projects: " << sqlite3_errmsg(sqlite_db)
                << std::endl;
    }
    sqlite3_finalize(stmt);
    loaded_ = true;
  }

  // [修改] 添加 override 关键字
  [[nodiscard]] auto GetPathParts(long long project_id) const
      -> std::vector<std::string> override {
    std::vector<std::string> parts;
    long long curr = project_id;
    while (curr != 0 && cache_.contains(curr)) {
      const auto& info = cache_.at(curr);
      parts.push_back(info.name);
      curr = info.parent_id;
    }
    std::ranges::reverse(parts);
    return parts;
  }

 private:
  ProjectNameCache() = default;
  bool loaded_ = false;
  std::unordered_map<long long, ProjectInfo> cache_;
};

#endif  // REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_