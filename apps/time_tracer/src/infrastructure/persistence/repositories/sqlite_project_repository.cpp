// infrastructure/persistence/repositories/sqlite_project_repository.cpp
#include "infrastructure/persistence/repositories/sqlite_project_repository.hpp"

#include <format>
#include <iostream>
#include <stdexcept>

#include "infrastructure/schema/time_records_schema.hpp"
SqliteProjectRepository::SqliteProjectRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

namespace {
constexpr size_t kDefaultProjectCapacity = 100;
}  // namespace

auto SqliteProjectRepository::GetAllProjects() -> std::vector<ProjectEntity> {
  DBManager db_manager(db_path_);
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database.");
  }
  sqlite3* db_connection = db_manager.GetDbConnection();

  const std::string sql = std::format(
      "SELECT {0}, {1}, {2} FROM {3} ORDER BY {2}",
      schema::projects::db::kId, schema::projects::db::kParentId,
      schema::projects::db::kName, schema::projects::db::kTable);
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db_connection, sql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error(std::string("Failed to prepare query: ") +
                             sqlite3_errmsg(db_connection));
  }

  std::vector<ProjectEntity> projects;
  projects.reserve(kDefaultProjectCapacity);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    ProjectEntity project;
    project.id = sqlite3_column_int(stmt, 0);
    if (sqlite3_column_type(stmt, 1) == SQLITE_NULL) {
      project.parent_id = std::nullopt;
    } else {
      project.parent_id = sqlite3_column_int(stmt, 1);
    }
    const char* name_ptr =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    project.name = (name_ptr != nullptr) ? name_ptr : "";
    projects.push_back(project);
  }
  sqlite3_finalize(stmt);

  return projects;
}
