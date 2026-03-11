class Statement {
 public:
  explicit Statement(sqlite3* sqlite_db);
  ~Statement();

  [[nodiscard]] auto GetInsertDayStmt() const -> sqlite3_stmt*;
  [[nodiscard]] auto GetInsertRecordStmt() const -> sqlite3_stmt*;
  [[nodiscard]] auto GetInsertProjectStmt() const -> sqlite3_stmt*;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_day_ = nullptr;
  sqlite3_stmt* stmt_insert_record_ = nullptr;
  sqlite3_stmt* stmt_insert_project_ = nullptr;

  auto PrepareStatements() -> void;
  auto FinalizeStatements() -> void;
};
