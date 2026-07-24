#ifndef DOMAIN_MODEL_GENERATED_EVENT_HPP_
#define DOMAIN_MODEL_GENERATED_EVENT_HPP_

#include <optional>
#include <string>

// GeneratedEvent is the internal semantic unit between timeline generation
// and TXT rendering. activity_token may be either an authored alias token or
// a canonical activity token, depending on generator mode.
enum class GeneratedEventKind { Point, Interval };

struct GeneratedEvent {
  GeneratedEventKind kind = GeneratedEventKind::Point;
  int start_minute = 0;
  int end_minute = 0;
  int start_second_of_day = -1;
  int end_second_of_day = -1;
  std::string activity_token;
  std::optional<std::string> remark_suffix;
};

#endif  // DOMAIN_MODEL_GENERATED_EVENT_HPP_
