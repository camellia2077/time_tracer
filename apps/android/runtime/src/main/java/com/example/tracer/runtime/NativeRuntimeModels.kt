package com.example.tracer

internal data class RuntimePaths(
    val dbPath: String,
    val outputRoot: String,
    val configRootPath: String,
    val configTomlPath: String,
    val inputRootPath: String,
    val cacheRootPath: String
)

internal data class NativeResponsePayload(
    val ok: Boolean,
    val content: String,
    val errorMessage: String,
    val reportHashSha256: String = "",
    val errorContract: ReportErrorContract? = null,
    val reportWindowMetadata: ReportWindowMetadata? = null
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
