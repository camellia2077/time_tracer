package com.example.tracer

data class TracerExchangePayloadItem(
    val relativePathHint: String,
    val content: String
)

data class TracerExchangeExportResult(
    val ok: Boolean,
    val message: String,
    val outputPath: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val converterFileCount: Int = 0,
    val manifestIncluded: Boolean = false
)

data class TracerExchangeImportResult(
    val ok: Boolean,
    val message: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val replacedMonthCount: Int = 0,
    val preservedMonthCount: Int = 0,
    val rebuiltMonthCount: Int = 0,
    val textRootUpdated: Boolean = false,
    val configApplied: Boolean = false,
    val databaseRebuilt: Boolean = false,
    val retainedFailureRoot: String = "",
    val backupRetainedRoot: String = "",
    val backupCleanupError: String = ""
)

data class TracerExchangeInspectResult(
    val ok: Boolean,
    val message: String,
    val renderedText: String,
    val inputPath: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val packageVersion: Int,
    val producerPlatform: String,
    val producerApp: String,
    val createdAtUtc: String
)
