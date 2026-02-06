// reports/shared/utils/format/year_utils.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_YEAR_UTILS_H_
#define REPORTS_SHARED_UTILS_FORMAT_YEAR_UTILS_H_

#include <string>
#include <string_view>

#include "reports/shared/api/shared_api.hpp"

// Gregorian year helper (calendar year).
REPORTS_SHARED_API auto ParseGregorianYear(std::string_view input,
                                           int& gregorian_year) -> bool;
REPORTS_SHARED_API auto FormatGregorianYear(int gregorian_year) -> std::string;

#endif  // REPORTS_SHARED_UTILS_FORMAT_YEAR_UTILS_H_
