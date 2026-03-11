module;

#include "domain/model/time_data_models.hpp"

export module tracer.core.domain.model.time_data_models;

export namespace tracer::core::domain::model {

#include "domain/detail/time_data_models_contract.inc"

}  // namespace tracer::core::domain::model

export namespace tracer::core::domain::modmodel {

using tracer::core::domain::model::ActivityStats;
using tracer::core::domain::model::BaseActivityRecord;

}  // namespace tracer::core::domain::modmodel
