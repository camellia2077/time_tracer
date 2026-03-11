module;

#include "infrastructure/config/models/app_config.hpp"

export module tracer.core.infrastructure.config.internal.config_detail_loader;

export namespace tracer::core::infrastructure::config::internal {

#include "infrastructure/config/detail/config_detail_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

export namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::LoadDetailedReports;

}  // namespace tracer::core::infrastructure::modconfig::internal
