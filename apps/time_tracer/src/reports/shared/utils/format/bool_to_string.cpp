// reports/shared/utils/format/bool_to_string.cpp
#include "bool_to_string.hpp"

auto bool_to_string(const std::string& val) -> std::string {
  return (val == "1") ? "true" : "false";
}