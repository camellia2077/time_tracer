module;

#include "shared/types/exceptions.hpp"

export module tracer.core.shared.exceptions;

export namespace tracer::core::shared::modtypes {

using ::tracer_core::common::AppError;
using ::tracer_core::common::ConfigError;
using ::tracer_core::common::DatabaseError;
using ::tracer_core::common::DllCompatibilityError;
using ::tracer_core::common::IoError;
using ::tracer_core::common::LogicError;

}  // namespace tracer::core::shared::modtypes
