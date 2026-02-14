// infrastructure/reports/shared/interfaces/formatter_c_report_data_bridge_internal.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_INTERNAL_H_
#define INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_INTERNAL_H_

#include <limits>
#include <string>

#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

namespace formatter_c_report_data_bridge::detail {

inline auto ValidateStringView(const TtStringView& view, const char* field_name,
                               bool allow_empty, std::string* error_message)
    -> bool {
  if (error_message == nullptr) {
    return false;
  }

  if (view.length == 0U) {
    if (!allow_empty) {
      *error_message =
          std::string("Field '") + field_name + "' must not be empty.";
      return false;
    }
    return true;
  }
  if (view.data == nullptr) {
    *error_message = std::string("Invalid string view for field '") +
                     field_name + "': data is null while length > 0.";
    return false;
  }
  if (view.length >
      static_cast<uint64_t>(std::numeric_limits<std::size_t>::max())) {
    *error_message =
        std::string("String length overflow for field '") + field_name + "'.";
    return false;
  }

  return true;
}

inline auto ParseStringView(const TtStringView& view, const char* field_name,
                            std::string* out_value, std::string* error_message)
    -> bool {
  if ((out_value == nullptr) || (error_message == nullptr)) {
    return false;
  }

  if (!ValidateStringView(view, field_name, true, error_message)) {
    return false;
  }
  if (view.length == 0U) {
    out_value->clear();
    return true;
  }

  out_value->assign(view.data, static_cast<std::size_t>(view.length));
  return true;
}

}  // namespace formatter_c_report_data_bridge::detail

#endif  // INFRASTRUCTURE_REPORTS_SHARED_INTERFACES_FORMATTER_C_REPORT_DATA_BRIDGE_INTERNAL_H_
