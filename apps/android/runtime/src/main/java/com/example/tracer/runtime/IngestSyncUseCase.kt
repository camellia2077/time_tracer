package com.example.tracer

internal class IngestSyncUseCase(
    private val autoSyncMaterializer: AutoSyncMaterializer,
    private val nativeIngest: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String
) {
    fun run(paths: RuntimePaths): String {
        autoSyncMaterializer.materialize(
            liveRawInputPath = paths.liveRawInputPath,
            liveAutoSyncInputPath = paths.liveAutoSyncInputPath
        )
        return nativeIngest(
            paths.liveAutoSyncInputPath,
            NativeBridge.DATE_CHECK_NONE,
            // Android build disables processed JSON output in core
            // (TT_ENABLE_PROCESSED_JSON_IO=OFF).
            false
        )
    }
}
