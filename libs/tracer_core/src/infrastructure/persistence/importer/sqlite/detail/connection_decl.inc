class Connection {
 public:
  explicit Connection(const std::string& db_path);
  ~Connection();

  [[nodiscard]] auto GetDb() const -> sqlite3*;
  auto BeginTransaction() -> bool;
  auto CommitTransaction() -> bool;
  auto RollbackTransaction() -> void;

 private:
  sqlite3* db_{nullptr};
};

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto ExecuteSql(sqlite3* sqlite_db, const std::string& sql_query,
                const std::string& error_context = "") -> bool;
// NOLINTEND(bugprone-easily-swappable-parameters)
