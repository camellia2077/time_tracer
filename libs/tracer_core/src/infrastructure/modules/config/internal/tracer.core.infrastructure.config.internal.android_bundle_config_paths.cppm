module;

#include <filesystem>
#include <optional>

export module tracer.core.infrastructure.config.internal.android_bundle_config_paths;

export namespace tracer::core::infrastructure::config::internal {

#include "infrastructure/config/internal/detail/android_bundle_config_paths_decl.inc"

}  // namespace tracer::core::infrastructure::config::internal

export namespace tracer::core::infrastructure::modconfig::internal {

using tracer::core::infrastructure::config::internal::AndroidBundleConfigPaths;
using tracer::core::infrastructure::config::internal::
    AndroidBundleReportConfigPathSet;
using tracer::core::infrastructure::config::internal::
    TryResolveAndroidBundleConfigPaths;

}  // namespace tracer::core::infrastructure::modconfig::internal
