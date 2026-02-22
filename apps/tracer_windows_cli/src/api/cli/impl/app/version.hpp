// api/cli/impl/app/version.hpp
#ifndef API_CLI_IMPL_APP_VERSION_H_
#define API_CLI_IMPL_APP_VERSION_H_

#include <string_view>

#include "shared/types/version.hpp"

#ifndef TRACER_WINDOWS_CLI_VERSION
#define TRACER_WINDOWS_CLI_VERSION "0.1.0"
#endif

#ifndef TRACER_WINDOWS_BUILD_TIMESTAMP
#define TRACER_WINDOWS_BUILD_TIMESTAMP "unknown-build-time"
#endif

namespace TracerWindowsVersion {

constexpr std::string_view kCliVersion = TRACER_WINDOWS_CLI_VERSION;
constexpr std::string_view kCliBuildTimestamp = TRACER_WINDOWS_BUILD_TIMESTAMP;
constexpr std::string_view kCoreVersion = AppInfo::kVersion;

} // namespace TracerWindowsVersion

#endif // API_CLI_IMPL_APP_VERSION_H_
