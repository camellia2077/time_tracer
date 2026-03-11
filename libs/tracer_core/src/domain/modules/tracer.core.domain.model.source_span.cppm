module;

#include "domain/model/source_span.hpp"

export module tracer.core.domain.model.source_span;

export namespace tracer::core::domain::model {

#include "domain/detail/source_span_contract.inc"

}  // namespace tracer::core::domain::model

export namespace tracer::core::domain::modmodel {

using tracer::core::domain::model::SourceSpan;

}  // namespace tracer::core::domain::modmodel
