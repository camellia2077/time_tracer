struct ImportProjectNode;

class ProjectResolver {
 public:
  explicit ProjectResolver(sqlite3* db_ptr, sqlite3_stmt* stmt_insert_project);
  ~ProjectResolver();

  auto PreloadAndResolve(const std::vector<std::string>& project_paths) -> void;

  [[nodiscard]] auto GetId(const std::string& project_path) const -> long long;

 private:
  sqlite3* db_;
  sqlite3_stmt* stmt_insert_project_;

  std::unique_ptr<ImportProjectNode> root_;
  std::unordered_map<std::string, long long> cache_;

  auto LoadFromDb() -> void;
  [[nodiscard]] auto EnsurePath(const std::string& path) -> long long;
};
