// api/cli/impl/utils/sqlite_utils.hpp
#pragma once

#include <string>
#include <utility>
#include <vector>

struct sqlite3;

namespace tracer_core::cli::impl::utils {

struct SqlParam {
  enum class Type { kText, kInt };
  Type type = Type::kText;
  std::string text_value;
  int int_value = 0;
};

auto EscapeLikeLiteral(const std::string &value) -> std::string;

auto BuildLikeContains(const std::string &value) -> std::string;

auto QueryStringColumn(sqlite3 *db_conn, const std::string &sql,
                       const std::vector<SqlParam> &params)
    -> std::vector<std::string>;

auto QueryYearMonth(sqlite3 *db_conn, const std::string &sql,
                    const std::vector<SqlParam> &params)
    -> std::vector<std::pair<int, int>>;

} // namespace tracer_core::cli::impl::utils
