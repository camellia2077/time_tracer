#ifndef INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_
#define INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_

#include "infra/query/data/internal/request.hpp"

namespace tracer::core::infrastructure::query::data::internal {

#include "infra/query/data/internal/detail/report_mapping_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::internal

namespace tracer::core::infrastructure::query::data::repository::internal {

using tracer::core::infrastructure::query::data::internal::
    BuildMappingNamesContent;
using tracer::core::infrastructure::query::data::internal::
    BuildMappingAliasKeysContent;
using tracer::core::infrastructure::query::data::internal::
    BuildWakeKeywordsContent;
using tracer::core::infrastructure::query::data::internal::
    BuildAuthorableEventTokensContent;
using tracer::core::infrastructure::query::data::internal::
    BuildReportChartContent;
using tracer::core::infrastructure::query::data::internal::
    BuildReportCompositionContent;
using tracer::core::infrastructure::query::data::internal::
    ValidateReportChartRequest;
using tracer::core::infrastructure::query::data::internal::
    ValidateReportCompositionRequest;

}  // namespace tracer::core::infrastructure::query::data::repository::internal

#endif  // INFRASTRUCTURE_QUERY_DATA_INTERNAL_REPORT_MAPPING_H_
