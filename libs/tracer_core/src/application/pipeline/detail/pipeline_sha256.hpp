#ifndef APPLICATION_PIPELINE_DETAIL_PIPELINE_SHA256_HPP_
#define APPLICATION_PIPELINE_DETAIL_PIPELINE_SHA256_HPP_

#include <cstdint>
#include <string>
#include <string_view>

namespace tracer::core::application::pipeline::detail {

[[nodiscard]] auto ComputeSha256Hex(std::string_view canonical_text)
    -> std::string;
[[nodiscard]] auto CurrentUnixMillis() -> std::int64_t;

}  // namespace tracer::core::application::pipeline::detail

#endif  // APPLICATION_PIPELINE_DETAIL_PIPELINE_SHA256_HPP_
