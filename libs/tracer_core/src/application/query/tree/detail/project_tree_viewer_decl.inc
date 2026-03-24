class ProjectTreeViewer {
 public:
  using ProjectRepositoryPtr = std::shared_ptr<IProjectRepository>;

  explicit ProjectTreeViewer(ProjectRepositoryPtr repository);
  ~ProjectTreeViewer();

  [[nodiscard]] auto GetRoots() -> std::vector<std::string>;
  [[nodiscard]] auto GetTree(const std::string& root_pattern, int max_depth)
      -> std::optional<std::vector<ProjectTreeNode>>;

 private:
  ProjectRepositoryPtr repository_;
};
