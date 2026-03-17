#ifndef HOST_CRYPTO_PROGRESS_BRIDGE_H_
#define HOST_CRYPTO_PROGRESS_BRIDGE_H_

#include <string>

namespace tracer_core::infrastructure::crypto {

struct FileCryptoProgressSnapshot;

}  // namespace tracer_core::infrastructure::crypto

namespace tracer_core::shell::crypto_progress_bridge {

[[nodiscard]] auto BuildProgressSnapshotJson(
    const tracer_core::infrastructure::crypto::FileCryptoProgressSnapshot&
        snapshot) -> std::string;

}  // namespace tracer_core::shell::crypto_progress_bridge

#endif  // HOST_CRYPTO_PROGRESS_BRIDGE_H_
