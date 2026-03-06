module;

#include "shared/utils/period_utils.hpp"

export module tracer.core.shared.period_utils;

export namespace tracer::core::shared::modperiod {

using ::FormatGregorianYear;
using ::FormatIsoWeek;
using ::IsoWeek;
using ::IsoWeekEndDate;
using ::IsoWeekFromDate;
using ::IsoWeekStartDate;
using ::ParseGregorianYear;
using ::ParseIsoWeek;

}  // namespace tracer::core::shared::modperiod
