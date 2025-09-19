// queries/shared/utils/db/query_utils.hpp
#ifndef QUERY_UTILS_HPP
#define QUERY_UTILS_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "common/utils/ProjectTree.hpp" 
#include "queries/shared/types/ReportFormat.hpp"

// --- Standalone Utility Functions ---

// Adds or subtracts days from a date string ("YYYYMMDD").
std::string add_days_to_date_str(std::string date_str, int days);

// Gets the current system date as "YYYYMMDD".
std::string get_current_date_str();

#endif // QUERY_UTILS_HPP