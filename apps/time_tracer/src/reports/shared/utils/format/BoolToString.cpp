// reports/shared/utils/format/BoolToString.cpp
#include "BoolToString.hpp"

std::string bool_to_string(const std::string& val) {
    return (val == "1") ? "true" : "false";
}