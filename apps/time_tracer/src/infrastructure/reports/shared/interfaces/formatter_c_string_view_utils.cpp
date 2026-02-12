// infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.cpp
#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

#include <limits>
#include <stdexcept>

namespace formatter_c_string_view_utils {

auto ToString(const TtStringView& view, const char* field_name) -> std::string {
  if (view.length == 0U) {
    return "";
  }
  if (view.data == nullptr) {
    throw std::invalid_argument(std::string("Invalid string view for field '") +
                                field_name +
                                "': data is null while length > 0.");
  }
  if (view.length >
      static_cast<uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::overflow_error(
        std::string("String length overflow for field '") + field_name + "'.");
  }
  return {view.data, static_cast<std::size_t>(view.length)};
}

auto BuildKeywordColorsMap(const TtFormatterKeywordColorV1* keyword_colors,
                           uint32_t keyword_color_count, const char* field_name)
    -> std::map<std::string, std::string> {
  std::map<std::string, std::string> color_map;
  if (keyword_color_count == 0U) {
    return color_map;
  }
  if (keyword_colors == nullptr) {
    throw std::invalid_argument(std::string(field_name) +
                                " is null while count > 0.");
  }

  for (uint32_t index = 0; index < keyword_color_count; ++index) {
    const auto kEy =
        ToString(keyword_colors[index].keyword, "keywordColors.keyword");
    const auto kValue =
        ToString(keyword_colors[index].color, "keywordColors.color");
    color_map[kEy] = kValue;
  }
  return color_map;
}

}  // namespace formatter_c_string_view_utils
