#ifndef HOST_TRACER_EXCHANGE_INSPECT_FORMATTER_HPP_
#define HOST_TRACER_EXCHANGE_INSPECT_FORMATTER_HPP_

#include <string>

#include "application/dto/core_responses.hpp"

namespace tracer_core::shell::tracer_exchange {

[[nodiscard]] auto BuildInspectContent(
    const tracer_core::core::dto::TracerExchangeInspectResult& inspect_result)
    -> std::string;

}  // namespace tracer_core::shell::tracer_exchange

#endif  // HOST_TRACER_EXCHANGE_INSPECT_FORMATTER_HPP_
