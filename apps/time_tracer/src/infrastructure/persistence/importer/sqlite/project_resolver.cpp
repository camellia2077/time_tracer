// infrastructure/persistence/importer/sqlite/project_resolver.cpp
#include "infrastructure/persistence/importer/sqlite/project_resolver.hpp"

#include <format>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "infrastructure/schema/sqlite_schema.hpp"
#include "shared/utils/string_utils.hpp"

namespace infrastructure::persistence::importer::sqlite {

struct ImportProjectNode {
  long long id = 0;
  std::string name;
  std::unordered_map<std::string, std::unique_ptr<ImportProjectNode>> children;
};

ProjectResolver::ProjectResolver(sqlite3* db_ptr,
                                 sqlite3_stmt* stmt_insert_project)
    : db_(db_ptr), stmt_insert_project_(stmt_insert_project) {}

ProjectResolver::~ProjectResolver() = default;

auto ProjectResolver::LoadFromDb() -> void {
  root_ = std::make_unique<ImportProjectNode>();
  root_->id = 0;

  const std::string kSql =
      std::format("SELECT {0}, {1}, {2} FROM {3}", schema::projects::db::kId,
                  schema::projects::db::kName, schema::projects::db::kParentId,
                  schema::projects::db::kTable);
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db_, kSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Error preparing load projects sql.");
  }

  struct Row {
    long long id;
    std::string name;
    long long parent_id;
  };
  std::vector<Row> all_rows;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    long long project_id = sqlite3_column_int64(stmt, 0);
    const unsigned char* name_ptr = sqlite3_column_text(stmt, 1);
    std::string name =
        (name_ptr != nullptr) ? reinterpret_cast<const char*>(name_ptr) : "";
    long long parent_id = 0;
    if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
      parent_id = sqlite3_column_int64(stmt, 2);
    }
    all_rows.push_back({project_id, name, parent_id});
  }
  sqlite3_finalize(stmt);

  std::unordered_map<long long, std::vector<Row>> adjacency_list;
  for (const auto& row : all_rows) {
    adjacency_list[row.parent_id].push_back(row);
  }

  std::queue<long long> queue;
  queue.push(0);

  std::unordered_map<long long, ImportProjectNode*> node_ptr_map;
  node_ptr_map[0] = root_.get();

  while (!queue.empty()) {
    long long current_parent_id = queue.front();
    queue.pop();

    ImportProjectNode* parent_node = node_ptr_map[current_parent_id];
    if (parent_node == nullptr) {
      continue;
    }

    if (adjacency_list.contains(current_parent_id)) {
      for (const auto& child_row : adjacency_list[current_parent_id]) {
        auto new_node = std::make_unique<ImportProjectNode>();
        new_node->id = child_row.id;
        new_node->name = child_row.name;

        ImportProjectNode* raw_ptr = new_node.get();
        parent_node->children[child_row.name] = std::move(new_node);
        node_ptr_map[child_row.id] = raw_ptr;
        queue.push(child_row.id);
      }
    }
  }
}

auto ProjectResolver::EnsurePath(const std::string& full_path) -> long long {
  std::vector<std::string> parts = SplitString(full_path, '_');

  ImportProjectNode* current_node = root_.get();
  long long current_parent_id = 0;

  for (const auto& part_name : parts) {
    auto child_it = current_node->children.find(part_name);

    if (child_it != current_node->children.end()) {
      current_node = child_it->second.get();
      current_parent_id = current_node->id;
    } else {
      sqlite3_reset(stmt_insert_project_);
      sqlite3_bind_text(stmt_insert_project_, 1, part_name.c_str(), -1,
                        SQLITE_TRANSIENT);

      if (current_parent_id == 0) {
        sqlite3_bind_null(stmt_insert_project_, 2);
      } else {
        sqlite3_bind_int64(stmt_insert_project_, 2, current_parent_id);
      }

      if (sqlite3_step(stmt_insert_project_) != SQLITE_DONE) {
        throw std::runtime_error("Error inserting project: " + part_name);
      }

      long long new_id = sqlite3_last_insert_rowid(db_);

      auto new_node = std::make_unique<ImportProjectNode>();
      new_node->id = new_id;
      new_node->name = part_name;

      ImportProjectNode* next_node = new_node.get();
      current_node->children[part_name] = std::move(new_node);

      current_node = next_node;
      current_parent_id = new_id;
    }
  }
  return current_parent_id;
}

auto ProjectResolver::PreloadAndResolve(
    const std::vector<std::string>& project_paths) -> void {
  if (cache_.empty()) {
    LoadFromDb();
  }

  for (const auto& path : project_paths) {
    if (!cache_.contains(path)) {
      long long project_id = EnsurePath(path);
      cache_[path] = project_id;
    }
  }
}

auto ProjectResolver::GetId(const std::string& project_path) const
    -> long long {
  auto cache_it = cache_.find(project_path);
  if (cache_it != cache_.end()) {
    return cache_it->second;
  }
  return 0;
}

}  // namespace infrastructure::persistence::importer::sqlite
