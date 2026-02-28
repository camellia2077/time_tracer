// shared/types/version.hpp
#ifndef SHARED_TYPES_VERSION_H_
#define SHARED_TYPES_VERSION_H_

#include <string_view>

#if defined(__has_include)
#if __has_include("tracer_core_build_metadata.hpp")
#include "tracer_core_build_metadata.hpp"
#endif
#endif

#ifndef TRACER_CORE_BUILD_DATE
#define TRACER_CORE_BUILD_DATE "unknown-build-date"
#endif

#ifndef TRACER_CORE_BUILD_TIMESTAMP
#define TRACER_CORE_BUILD_TIMESTAMP "unknown-build-time"
#endif

// 使用命名空间来组织应用配置信息，避免全局污染
namespace AppInfo {
// 使用 constexpr 和 string_view 可以在编译期确定字符串，效率更高
// 这也是现代C++的推荐做法
constexpr std::string_view kVersion = "0.7.0";
constexpr std::string_view kLastUpdated = TRACER_CORE_BUILD_TIMESTAMP;

}  // namespace AppInfo

#endif  // SHARED_TYPES_VERSION_H_
