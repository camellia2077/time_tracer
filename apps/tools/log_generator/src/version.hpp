// version.hpp
#ifndef VERSION_H_
#define VERSION_H_

#include <string_view>

#if defined(__has_include)
#if __has_include("log_generator_build_metadata.hpp")
#include "log_generator_build_metadata.hpp"
#endif
#endif

#ifndef LOG_GENERATOR_BUILD_TIMESTAMP
#define LOG_GENERATOR_BUILD_TIMESTAMP "unknown-build-time"
#endif

namespace AppVersion {
inline constexpr std::string_view APP_VERSION = "0.1.5";
inline constexpr std::string_view LAST_UPDATE = LOG_GENERATOR_BUILD_TIMESTAMP;
}  // namespace AppVersion

#endif  // VERSION_H_
