package com.example.tracer

internal class RuntimeSession(
    private val runtimeEnvironment: RuntimeEnvironment,
    private val rawRecordStore: LiveRawRecordStore
) {
    private var runtimePaths: RuntimePaths? = null
    private var textStorage: TextStorage? = null
    private var configTomlStorage: ConfigTomlStorage? = null

    fun runtimePathsOrNull(): RuntimePaths? = runtimePaths

    fun ensureRuntimePaths(): RuntimePaths = ensureRuntimePathsCached(
        existing = runtimePaths,
        runtimeEnvironment = runtimeEnvironment,
        onPrepared = { runtimePaths = it }
    )

    fun ensureTextStorage(): TextStorage = ensureTextStorageCached(
        existing = textStorage,
        runtimePathsProvider = ::ensureRuntimePaths,
        rawRecordStore = rawRecordStore,
        onPrepared = { textStorage = it }
    )

    fun ensureConfigTomlStorage(): ConfigTomlStorage = ensureConfigTomlStorageCached(
        existing = configTomlStorage,
        runtimePathsProvider = ::ensureRuntimePaths,
        onPrepared = { configTomlStorage = it }
    )

    fun reset() {
        resetRuntimeStorageCaches(
            onRuntimePathsReset = { runtimePaths = null },
            onTextStorageReset = { textStorage = null },
            onConfigTomlStorageReset = { configTomlStorage = null }
        )
    }
}
