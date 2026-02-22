package com.example.tracer

internal fun ensureRuntimePathsCached(
    existing: RuntimePaths?,
    runtimeEnvironment: RuntimeEnvironment,
    onPrepared: (RuntimePaths) -> Unit
): RuntimePaths {
    if (existing != null) {
        return existing
    }

    val prepared = runtimeEnvironment.prepareRuntimePaths()
    onPrepared(prepared)
    return prepared
}

internal fun ensureTextStorageCached(
    existing: TextStorage?,
    runtimePathsProvider: () -> RuntimePaths,
    rawRecordStore: LiveRawRecordStore,
    onPrepared: (TextStorage) -> Unit
): TextStorage {
    if (existing != null) {
        return existing
    }

    val paths = runtimePathsProvider()
    val created = MultiInputTextStorage(
        fullInputPath = paths.fullInputPath,
        smokeInputPath = paths.smokeInputPath,
        liveRawInputPath = paths.liveRawInputPath,
        recordStore = rawRecordStore
    )
    onPrepared(created)
    return created
}

internal fun ensureConfigTomlStorageCached(
    existing: ConfigTomlStorage?,
    runtimePathsProvider: () -> RuntimePaths,
    onPrepared: (ConfigTomlStorage) -> Unit
): ConfigTomlStorage {
    if (existing != null) {
        return existing
    }

    val paths = runtimePathsProvider()
    val created = ConfigTomlStorage(paths.configRootPath)
    onPrepared(created)
    return created
}

internal fun resetRuntimeStorageCaches(
    onRuntimePathsReset: () -> Unit,
    onTextStorageReset: () -> Unit,
    onConfigTomlStorageReset: () -> Unit
) {
    onRuntimePathsReset()
    onTextStorageReset()
    onConfigTomlStorageReset()
}
