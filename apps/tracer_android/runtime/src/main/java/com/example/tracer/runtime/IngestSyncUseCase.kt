package com.example.tracer

internal class IngestSyncUseCase(
    private val autoSyncMaterializer: AutoSyncMaterializer
) {
    fun run(paths: RuntimePaths): String {
        autoSyncMaterializer.materialize(
            liveRawInputPath = paths.liveRawInputPath,
            liveAutoSyncInputPath = paths.liveAutoSyncInputPath
        )
        return NativeBridge.nativeIngest(
            inputPath = paths.liveAutoSyncInputPath,
            dateCheckMode = NativeBridge.DATE_CHECK_NONE,
            saveProcessedOutput = false
        )
    }
}
