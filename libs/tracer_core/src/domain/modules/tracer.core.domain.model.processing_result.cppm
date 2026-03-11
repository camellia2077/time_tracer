module;

#include "domain/model/processing_result.hpp"

export module tracer.core.domain.model.processing_result;

export namespace tracer::core::domain::model {

#include "domain/detail/processing_result_contract.inc"

}  // namespace tracer::core::domain::model

export namespace tracer::core::domain::modmodel {

using tracer::core::domain::model::ProcessingResult;
using tracer::core::domain::model::ProcessingTimings;

}  // namespace tracer::core::domain::modmodel
