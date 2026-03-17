module;

#include "shared/utils/string_utils.hpp"

export module tracer.core.shared.string_utils;

export namespace tracer::core::shared::string_utils {

using ::tracer::core::shared::string_utils::SplitString;
using ::tracer::core::shared::string_utils::Trim;

}  // namespace tracer::core::shared::string_utils

export namespace tracer::core::shared::modutils {

using tracer::core::shared::string_utils::SplitString;
using tracer::core::shared::string_utils::Trim;

}  // namespace tracer::core::shared::modutils
