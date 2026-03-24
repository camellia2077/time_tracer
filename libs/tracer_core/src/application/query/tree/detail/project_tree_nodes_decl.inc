[[nodiscard]] auto BuildProjectTreeNodesFromReportTree(
    const ::reporting::ProjectTree& tree) -> std::vector<ProjectTreeNode>;

[[nodiscard]] auto FindProjectTreeNodesByPath(
    const std::vector<ProjectTreeNode>& roots, std::string_view root_pattern)
    -> std::vector<ProjectTreeNode>;

[[nodiscard]] auto LimitProjectTreeDepth(
    const std::vector<ProjectTreeNode>& roots, int max_depth)
    -> std::vector<ProjectTreeNode>;
