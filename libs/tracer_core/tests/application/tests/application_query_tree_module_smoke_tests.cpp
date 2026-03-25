import tracer.core.application;

#include <iostream>
#include <optional>
#include <string_view>
#include <type_traits>

namespace {

namespace app_tree = tracer::core::application::query::tree;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto RunQueryTreeModuleSmoke(int& failures) -> void {
  Expect(std::is_class_v<app_tree::ProjectTreeNode>,
         "ProjectTreeNode should be visible through query tree module.",
         failures);
  Expect(std::is_class_v<app_tree::ProjectTreeViewer>,
         "ProjectTreeViewer should be visible through query tree module.",
         failures);

  app_tree::ProjectTreeNode root_node{};
  root_node.name = "root";
  root_node.path = "root";
  root_node.duration_seconds = 42;
  root_node.children.push_back(
      {.name = "child", .path = "root/child", .duration_seconds = 7});
  Expect(root_node.name == "root" &&
             root_node.duration_seconds == std::optional<long long>(42) &&
             root_node.children.size() == 1U &&
             root_node.children.front().name == "child",
         "ProjectTreeNode fields should remain writable.", failures);

  const auto build_nodes_fn = &app_tree::BuildProjectTreeNodesFromReportTree;
  const auto find_nodes_fn = &app_tree::FindProjectTreeNodesByPath;
  const auto limit_depth_fn = &app_tree::LimitProjectTreeDepth;
  Expect(build_nodes_fn != nullptr,
         "BuildProjectTreeNodesFromReportTree should be exported.", failures);
  Expect(find_nodes_fn != nullptr,
         "FindProjectTreeNodesByPath should be exported.", failures);
  Expect(limit_depth_fn != nullptr, "LimitProjectTreeDepth should be exported.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  RunQueryTreeModuleSmoke(failures);
  if (failures == 0) {
    std::cout
        << "[PASS] tracer_core_application_query_tree_module_smoke_tests\n";
    return 0;
  }
  std::cerr << "[FAIL] tracer_core_application_query_tree_module_smoke_tests "
               "failures: "
            << failures << '\n';
  return 1;
}
