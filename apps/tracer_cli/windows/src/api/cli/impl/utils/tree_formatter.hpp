// api/cli/impl/utils/tree_formatter.hpp
#ifndef API_CLI_IMPL_UTILS_TREE_FORMATTER_H_
#define API_CLI_IMPL_UTILS_TREE_FORMATTER_H_

#include <string>
#include <vector>

#include "application/reporting/tree/project_tree_data.hpp"

/**
 * @brief 项目树输出格式化器，负责将树数据结构输出到控制台
 */
class TreeFormatter {
public:
  /**
   * @brief 输出根项目列表
   */
  static auto PrintRoots(const std::vector<std::string> &roots) -> void;

  /**
   * @brief 输出树形结构
   */
  static auto PrintTree(const std::vector<ProjectTreeNode> &nodes) -> void;
};

namespace tracer_core::cli::impl::utils {

struct DateParts {
  int year;
  int month;
  int day;
};

struct MonthInfo {
  int year;
  int month;
};

auto IsLeapYear(int year) -> bool;

auto DaysInMonth(const MonthInfo &info) -> int;

auto FormatDate(const DateParts &parts) -> std::string;

auto NormalizeDateInput(const std::string &input, bool is_end) -> std::string;

} // namespace tracer_core::cli::impl::utils

#endif // API_CLI_IMPL_UTILS_TREE_FORMATTER_H_
