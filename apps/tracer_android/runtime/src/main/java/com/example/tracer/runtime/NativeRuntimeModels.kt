package com.example.tracer

internal data class RuntimePaths(
    val dbPath: String,
    val outputRoot: String,
    val configRootPath: String,
    val configTomlPath: String,
    val smokeInputPath: String,
    val fullInputPath: String,
    val liveRawInputPath: String,
    val liveAutoSyncInputPath: String
)

internal data class NativeResponsePayload(
    val ok: Boolean,
    val content: String,
    val errorMessage: String
)

internal data class RecordWriteSnapshot(
    val monthFile: java.io.File,
    val logicalDate: String,
    val dayMarker: String,
    val eventTime: String,
    val warnings: List<String>
)

internal data class EnsureMonthFileResult(
    val monthFile: java.io.File,
    val created: Boolean
)
