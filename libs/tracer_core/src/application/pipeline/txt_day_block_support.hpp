#ifndef APPLICATION_PIPELINE_TXT_DAY_BLOCK_SUPPORT_HPP_
#define APPLICATION_PIPELINE_TXT_DAY_BLOCK_SUPPORT_HPP_

#include "domain/types/converter_config.hpp"
#include "application/dto/pipeline_requests.hpp"
#include "application/dto/pipeline_responses.hpp"

namespace tracer::core::application::pipeline::txt_day_block {

[[nodiscard]] auto DefaultDayMarker(
    const tracer_core::core::dto::DefaultTxtDayMarkerRequest& request)
    -> tracer_core::core::dto::DefaultTxtDayMarkerResponse;

[[nodiscard]] auto ResolveDayBlock(
    const tracer_core::core::dto::ResolveTxtDayBlockRequest& request)
    -> tracer_core::core::dto::ResolveTxtDayBlockResponse;

[[nodiscard]] auto ReplaceDayBlock(
    const tracer_core::core::dto::ReplaceTxtDayBlockRequest& request)
    -> tracer_core::core::dto::ReplaceTxtDayBlockResponse;

[[nodiscard]] auto ResolveDayEdit(
    const tracer_core::core::dto::ResolveTxtDayEditRequest& request,
    const ConverterConfig& config)
    -> tracer_core::core::dto::ResolveTxtDayEditResponse;

[[nodiscard]] auto ApplyDayEdit(
    const tracer_core::core::dto::ApplyTxtDayEditRequest& request,
    const ConverterConfig& config)
    -> tracer_core::core::dto::ApplyTxtDayEditResponse;

}  // namespace tracer::core::application::pipeline::txt_day_block

#endif  // APPLICATION_PIPELINE_TXT_DAY_BLOCK_SUPPORT_HPP_
