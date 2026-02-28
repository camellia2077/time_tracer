// domain/model/source_span.hpp
#ifndef DOMAIN_MODEL_SOURCE_SPAN_H_
#define DOMAIN_MODEL_SOURCE_SPAN_H_

#include <string>

struct SourceSpan {
  std::string file_path;
  int line_start = 0;
  int line_end = 0;
  int column_start = 0;
  int column_end = 0;
  std::string raw_text;

  [[nodiscard]] auto HasLine() const -> bool { return line_start > 0; }
};

#endif  // DOMAIN_MODEL_SOURCE_SPAN_H_
