package com.example.tracer

import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import org.json.JSONObject

internal val runtimeCryptoCallMutex = Mutex()

internal suspend fun <T> executeWithCryptoProgressListener(
    onProgress: ((FileCryptoProgressEvent) -> Unit)?,
    setProgressListener: (((String) -> Unit)?) -> Unit,
    block: () -> T
): T = runtimeCryptoCallMutex.withLock {
    setProgressListener(
        if (onProgress == null) {
            null
        } else {
            { rawProgress ->
                parseCryptoProgressEvent(rawProgress)?.let(onProgress)
            }
        }
    )
    try {
        block()
    } finally {
        setProgressListener(null)
    }
}

internal fun parseCryptoProgressEvent(rawProgress: String): FileCryptoProgressEvent? {
    return runCatching {
        val json = JSONObject(rawProgress)
        FileCryptoProgressEvent(
            operation = FileCryptoOperation.fromWireValue(json.optString("operation")),
            phase = FileCryptoPhase.fromWireValue(json.optString("phase")),
            currentGroupLabel = json.optString("current_group_label"),
            groupIndex = json.optInt("group_index", 0),
            groupCount = json.optInt("group_count", 0),
            fileIndexInGroup = json.optInt("file_index_in_group", 0),
            fileCountInGroup = json.optInt("file_count_in_group", 0),
            currentFileIndex = json.optInt("current_file_index", 0),
            totalFiles = json.optInt("total_files", 0),
            currentFileDoneBytes = json.optLong("current_file_done_bytes", 0L),
            currentFileTotalBytes = json.optLong("current_file_total_bytes", 0L),
            overallDoneBytes = json.optLong("overall_done_bytes", 0L),
            overallTotalBytes = json.optLong("overall_total_bytes", 0L),
            speedBytesPerSec = json.optLong("speed_bytes_per_sec", 0L),
            remainingBytes = json.optLong("remaining_bytes", 0L),
            etaSeconds = json.optLong("eta_seconds", 0L)
        )
    }.getOrNull()
}
