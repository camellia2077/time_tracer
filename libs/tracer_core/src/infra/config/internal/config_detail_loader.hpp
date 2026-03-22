// infra/config/internal/config_detail_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_DETAIL_LOADER_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_DETAIL_LOADER_H_

#include "infra/config/models/app_config.hpp"

namespace tracer::core::infrastructure::config::internal {

#include "infra/config/internal/detail/config_detail_loader_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace ConfigDetailLoader {

using tracer::core::infrastructure::config::internal::LoadDetailedReports;

}  // namespace ConfigDetailLoader

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_DETAIL_LOADER_H_
