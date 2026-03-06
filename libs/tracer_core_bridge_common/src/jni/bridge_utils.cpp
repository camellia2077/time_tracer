#include "jni/bridge_utils.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "tracer/transport/runtime_codec.hpp"

namespace tracer_core_bridge_common::jni {

namespace tt_transport = tracer::transport;

[[nodiscard]] auto BuildResponseJson(bool ok, std::string_view error_message,
                                     std::string_view content) -> std::string {
  return tt_transport::SerializeResponseEnvelope(
      tt_transport::BuildResponseEnvelope(ok, error_message, content));
}

[[nodiscard]] auto ParseDateCheckModeCode(int value) -> std::string {
  if (value == 0) {
    return "none";
  }
  if (value == 1) {
    return "continuity";
  }
  if (value == 2) {
    return "full";
  }
  throw std::invalid_argument("Unsupported date_check_mode code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseDataQueryActionCode(int value) -> std::string {
  if (value == 0) {
    return "years";
  }
  if (value == 1) {
    return "months";
  }
  if (value == 2) {
    return "days";
  }
  if (value == 3) {
    return "days_duration";
  }
  if (value == 4) {
    return "days_stats";
  }
  if (value == 5) {
    return "search";
  }
  if (value == 6) {
    return "activity_suggest";
  }
  if (value == 7) {
    return "tree";
  }
  if (value == 8) {
    return "mapping_names";
  }
  if (value == 9) {
    return "report_chart";
  }
  throw std::invalid_argument("Unsupported query action code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportTypeCode(int value) -> std::string {
  if (value == 0) {
    return "day";
  }
  if (value == 1) {
    return "month";
  }
  if (value == 2) {
    return "recent";
  }
  if (value == 3) {
    return "week";
  }
  if (value == 4) {
    return "year";
  }
  if (value == 5) {
    return "range";
  }
  throw std::invalid_argument("Unsupported report type code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseReportFormatCode(int value) -> std::string {
  if (value == 0) {
    return "markdown";
  }
  if (value == 1) {
    return "latex";
  }
  if (value == 2) {
    return "typst";
  }
  throw std::invalid_argument("Unsupported report format code: " +
                              std::to_string(value));
}

[[nodiscard]] auto ParseCoreResponse(const char* response_json,
                                     std::string_view context)
    -> tt_transport::ResponseEnvelope {
  const auto parsed = tt_transport::ParseResponseEnvelope(
      response_json != nullptr ? std::string_view(response_json)
                               : std::string_view{},
      context);
  if (parsed.HasError()) {
    throw std::runtime_error(parsed.error.message);
  }
  return parsed.envelope;
}

}  // namespace tracer_core_bridge_common::jni
