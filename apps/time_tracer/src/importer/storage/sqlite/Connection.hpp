// importer/storage/sqlite/Connection.hpp
#ifndef IMPORTER_STORAGE_SQLITE_CONNECTION_H_
#define IMPORTER_STORAGE_SQLITE_CONNECTION_H_

#include <sqlite3.h>

#include <string>

class Connection {
 public:
  explicit Connection(const std::string& db_path);
  ~Connection();

  sqlite3* get_db() const;
  bool begin_transaction();
  bool commit_transaction();
  void rollback_transaction();

 private:
  sqlite3* db_;  // MODIFIED
};

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
bool execute_sql(sqlite3* sqlite_db, const std::string& sql_query,
                 const std::string& error_context = "");
// NOLINTEND(bugprone-easily-swappable-parameters)

#endif  // IMPORTER_STORAGE_SQLITE_CONNECTION_H_
