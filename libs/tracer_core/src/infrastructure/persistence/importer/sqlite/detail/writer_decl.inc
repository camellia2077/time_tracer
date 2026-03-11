class ProjectResolver;

class Writer {
 public:
  // NOLINTBEGIN(bugprone-easily-swappable-parameters)
  explicit Writer(sqlite3* sqlite_db, sqlite3_stmt* stmt_day,
                  sqlite3_stmt* stmt_record, sqlite3_stmt* stmt_insert_project);
  // NOLINTEND(bugprone-easily-swappable-parameters)

  ~Writer();

  auto InsertDays(const std::vector<DayData>& days) -> void;
  auto InsertRecords(const std::vector<TimeRecordInternal>& records) -> void;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_day_;
  sqlite3_stmt* stmt_insert_record_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ProjectResolver> project_resolver_;
};
