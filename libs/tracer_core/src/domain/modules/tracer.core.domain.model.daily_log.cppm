module;

#include "domain/model/daily_log.hpp"

export module tracer.core.domain.model.daily_log;

export namespace tracer::core::domain::model {

#include "domain/detail/daily_log_contract.inc"

}  // namespace tracer::core::domain::model

export namespace tracer::core::domain::modmodel {

using tracer::core::domain::model::DailyLog;
using tracer::core::domain::model::RawEvent;

}  // namespace tracer::core::domain::modmodel
