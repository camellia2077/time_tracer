// reports/shared/utils/format/bool_to_string.cpp
#include "reports/shared/utils/format/bool_to_string.hpp"

auto BoolToString(const std::string& val) -> std::string {
  return (val == "1") ? "true" : "false";
}