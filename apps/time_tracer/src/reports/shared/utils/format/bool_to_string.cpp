// reports/shared/utils/format/bool_to_string.cpp
#include "bool_to_string.hpp"

std::string bool_to_string(const std::string& val) {
    return (val == "1") ? "true" : "false";
}