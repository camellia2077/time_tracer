// infrastructure/config/internal/android_bundle_config_paths.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_ANDROID_BUNDLE_CONFIG_PATHS_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_ANDROID_BUNDLE_CONFIG_PATHS_H_

#include <filesystem>
#include <optional>

namespace tracer::core::infrastructure::config::internal {

#include "infrastructure/config/internal/detail/android_bundle_config_paths_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::AndroidBundleConfigPaths;
using tracer::core::infrastructure::config::internal::
    AndroidBundleReportConfigPathSet;
using tracer::core::infrastructure::config::internal::
    TryResolveAndroidBundleConfigPaths;

}  // namespace tracer::core::infrastructure::modconfig::internal

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_ANDROID_BUNDLE_CONFIG_PATHS_H_
