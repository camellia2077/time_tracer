class SqliteProjectRepository final : public IProjectRepository {
 public:
  explicit SqliteProjectRepository(std::string db_path);

  auto GetAllProjects() -> std::vector<ProjectEntity> override;

 private:
  std::string db_path_;
};
