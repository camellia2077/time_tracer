// shared/utils/ide_location_formatter.hpp
#ifndef SHARED_UTILS_IDE_LOCATION_FORMATTER_H_
#define SHARED_UTILS_IDE_LOCATION_FORMATTER_H_

#include <string>
#include <string_view>

namespace tracer::core::shared::ide_location {

[[nodiscard]] inline auto BuildIdeLocationPrefix(std::string_view file_path,
                                                 int line_start,
                                                 int line_end = 0)
    -> std::string {
  if (!file_path.empty()) {
    std::string prefix(file_path);
    prefix.push_back(':');

    if (line_start > 0) {
      prefix += std::to_string(line_start);
      if (line_end > line_start) {
        prefix.push_back('-');
        prefix += std::to_string(line_end);
      }
      prefix += ": ";
      return prefix;
    }

    prefix.push_back(' ');
    return prefix;
  }

  if (line_start > 0) {
    std::string prefix = "Line " + std::to_string(line_start);
    if (line_end > line_start) {
      prefix.push_back('-');
      prefix += std::to_string(line_end);
    }
    prefix += ": ";
    return prefix;
  }

  return "";
}

template <typename SpanLike>
[[nodiscard]] auto BuildIdeLocationPrefix(const SpanLike& span,
                                          std::string_view fallback_file = {})
    -> std::string {
  if (!span.file_path.empty()) {
    return BuildIdeLocationPrefix(span.file_path, span.line_start, span.line_end);
  }
  return BuildIdeLocationPrefix(fallback_file, span.line_start, span.line_end);
}

}  // namespace tracer::core::shared::ide_location

#endif  // SHARED_UTILS_IDE_LOCATION_FORMATTER_H_
