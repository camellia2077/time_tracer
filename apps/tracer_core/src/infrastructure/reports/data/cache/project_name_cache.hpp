// infrastructure/reports/data/cache/project_name_cache.hpp
#ifndef INFRASTRUCTURE_REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_
#define INFRASTRUCTURE_REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_

#include <sqlite3.h>

#include <algorithm>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain/ports/diagnostics.hpp"
#include "domain/reports/interfaces/i_project_info_provider.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

struct ProjectInfo {
  std::string name;
  long long parent_id;
};

class ProjectNameCache : public IProjectInfoProvider {
 public:
  ProjectNameCache() = default;

  auto EnsureLoaded(sqlite3* sqlite_db)
      -> void override {  // NOLINT(readability-function-cognitive-complexity)
    if (sqlite_db == nullptr) {
      tracer_core::domain::ports::EmitError(
          "Failed to load projects: database handle is null.");
      return;
    }

    // Always refresh from DB.
    //
    // Android runtime is long-lived and allows multiple ingest sessions in one
    // process. If we keep a one-time singleton snapshot, project_id -> name
    // can become stale after ingest/replace operations and corrupt report
    // project breakdown output.
    cache_.clear();

    const std::string kSql = std::format(
        "SELECT {0}, {1}, {2} FROM {3}", schema::projects::db::kId,
        schema::projects::db::kName, schema::projects::db::kParentId,
        schema::projects::db::kTable);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(sqlite_db, kSql.c_str(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
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
      tracer_core::domain::ports::EmitError(
          "Failed to load projects: " + std::string(sqlite3_errmsg(sqlite_db)));
    }
    sqlite3_finalize(stmt);
  }

  void Invalidate() { cache_.clear(); }

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
  std::unordered_map<long long, ProjectInfo> cache_;
};

#endif  // INFRASTRUCTURE_REPORTS_DATA_CACHE_PROJECT_NAME_CACHE_H_
