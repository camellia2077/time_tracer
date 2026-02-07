// api/cli/impl/utils/sqlite_utils.cpp
#include "api/cli/impl/utils/sqlite_utils.hpp"

#include <sqlite3.h>

#include <stdexcept>

namespace time_tracer::cli::impl::utils {

auto EscapeLikeLiteral(const std::string& value) -> std::string {
  std::string escaped;
  escaped.reserve(value.size());
  for (char character : value) {
    if (character == '%' || character == '_' || character == '\\') {
      escaped.push_back('\\');
    }
    escaped.push_back(character);
  }
  return escaped;
}

auto BuildLikeContains(const std::string& value) -> std::string {
  return "%" + EscapeLikeLiteral(value) + "%";
}

auto QueryStringColumn(sqlite3* db_conn, const std::string& sql,
                       const std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == SqlParam::Type::kInt) {
      sqlite3_bind_int(stmt, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(stmt, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      results.emplace_back(reinterpret_cast<const char*>(text));
    }
  }
  sqlite3_finalize(stmt);
  return results;
}

auto QueryYearMonth(sqlite3* db_conn, const std::string& sql,
                    const std::vector<SqlParam>& params)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == SqlParam::Type::kInt) {
      sqlite3_bind_int(stmt, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(stmt, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int year = sqlite3_column_int(stmt, 0);
    int month = sqlite3_column_int(stmt, 1);
    results.emplace_back(year, month);
  }
  sqlite3_finalize(stmt);
  return results;
}

}  // namespace time_tracer::cli::impl::utils
