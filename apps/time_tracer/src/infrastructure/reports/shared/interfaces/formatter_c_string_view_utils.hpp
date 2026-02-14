// infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_STRING_VIEW_UTILS_H_
#define INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_STRING_VIEW_UTILS_H_

#include <map>
#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace formatter_c_string_view_utils {

REPORTS_SHARED_API auto ToString(const TtStringView& view,
                                 const char* field_name) -> std::string;

REPORTS_SHARED_API auto BuildKeywordColorsMap(
    const TtFormatterKeywordColorV1* keyword_colors,
    uint32_t keyword_color_count, const char* field_name)
    -> std::map<std::string, std::string>;

}  // namespace formatter_c_string_view_utils

#endif  // INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_STRING_VIEW_UTILS_H_
