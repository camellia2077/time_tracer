// domain/types/time_order_mode.hpp
#ifndef DOMAIN_TYPES_TIME_ORDER_MODE_H_
#define DOMAIN_TYPES_TIME_ORDER_MODE_H_

// Atomic record time ordering mode.
// This controls how HHmm is projected onto a comparable timeline.
enum class TimeOrderMode {
  kStrictCalendar = 0,
  kLogicalDay0600 = 1,
};

namespace tracer::core::domain::types {

#include "domain/detail/time_order_mode_contract.inc"

}  // namespace tracer::core::domain::types

#endif  // DOMAIN_TYPES_TIME_ORDER_MODE_H_
