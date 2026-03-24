#ifndef APPLICATION_DTO_SHARED_ENVELOPES_HPP_
#define APPLICATION_DTO_SHARED_ENVELOPES_HPP_

#include <string>

namespace tracer_core::core::dto {

// Shared aggregate envelopes intentionally remain outside capability-owned DTO
// families because multiple application APIs and shell/runtime bridges reuse
// them directly.
struct OperationAck {
  bool ok = true;
  std::string error_message;
};

struct TextOutput {
  bool ok = true;
  std::string content;
  std::string error_message;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_SHARED_ENVELOPES_HPP_
