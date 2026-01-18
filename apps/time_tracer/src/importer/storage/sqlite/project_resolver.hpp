// importer/storage/sqlite/project_resolver.hpp
#ifndef IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_HPP_
#define IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sqlite3.h>

struct ImportProjectNode; // Forward declaration

class ProjectResolver {
public:
    explicit ProjectResolver(sqlite3* db, sqlite3_stmt* stmt_insert_project);
    ~ProjectResolver();

    // Batch preload and resolve
    void preload_and_resolve(const std::vector<std::string>& project_paths);
    
    // Get ID from cache
    long long get_id(const std::string& project_path) const;

private:
    sqlite3* db_;
    sqlite3_stmt* stmt_insert_project_;

    std::unique_ptr<ImportProjectNode> root_;
    std::unordered_map<std::string, long long> cache_;

    void load_from_db();
    long long ensure_path(const std::string& path);
};

#endif // IMPORTER_STORAGE_SQLITE_PROJECT_RESOLVER_HPP_