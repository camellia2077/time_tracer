// importer/storage/sqlite/connection.hpp
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

bool execute_sql(sqlite3* db, const std::string& sql,
                 const std::string& context_msg = "");

#endif  // IMPORTER_STORAGE_SQLITE_CONNECTION_H_