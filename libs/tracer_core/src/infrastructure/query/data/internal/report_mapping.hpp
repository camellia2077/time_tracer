#ifndef INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_
#define INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_

#include "infrastructure/query/data/internal/request.hpp"

namespace tracer::core::infrastructure::query::data::internal {

#include "infrastructure/query/data/internal/detail/report_mapping_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::internal

namespace infrastructure::persistence::data_query_service_internal {

using tracer::core::infrastructure::query::data::internal::BuildMappingNamesContent;
using tracer::core::infrastructure::query::data::internal::BuildReportChartContent;
using tracer::core::infrastructure::query::data::internal::ValidateReportChartRequest;

}  // namespace infrastructure::persistence::data_query_service_internal

#endif  // INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_
