#ifndef HOST_NATIVE_BRIDGE_PROGRESS_HPP_
#define HOST_NATIVE_BRIDGE_PROGRESS_HPP_

#include <jni.h>

#include "application/dto/exchange_requests.hpp"

namespace tracer_core::api::android::bridge_internal {

auto BuildTracerExchangeProgressObserver(JNIEnv* env)
    -> tracer_core::core::dto::TracerExchangeProgressObserver;

}  // namespace tracer_core::api::android::bridge_internal

#endif  // HOST_NATIVE_BRIDGE_PROGRESS_HPP_
