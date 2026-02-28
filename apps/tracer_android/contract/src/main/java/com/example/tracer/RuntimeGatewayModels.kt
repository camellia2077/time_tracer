package com.example.tracer

data class NativeCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = ""
)

data class ReportCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val outputText: String,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = ""
)

data class ClearAndInitResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val clearMessage: String,
    val initResponse: String,
    val operationId: String = ""
)

data class ClearTxtResult(
    val ok: Boolean,
    val message: String
)

data class RecordActionResult(
    val ok: Boolean,
    val message: String,
    val operationId: String = ""
)

data class TxtHistoryListResult(
    val ok: Boolean,
    val files: List<String>,
    val message: String
)

data class TxtFileContentResult(
    val ok: Boolean,
    val filePath: String,
    val content: String,
    val message: String
)

data class ConfigTomlListResult(
    val ok: Boolean,
    val converterFiles: List<String>,
    val reportFiles: List<String>,
    val message: String
)

data class ActivitySuggestionResult(
    val ok: Boolean,
    val suggestions: List<String>,
    val message: String,
    val operationId: String = ""
)

data class ActivityMappingNamesResult(
    val ok: Boolean,
    val names: List<String>,
    val message: String,
    val operationId: String = ""
)

enum class DataTreePeriod(val wireValue: String) {
    DAY("day"),
    WEEK("week"),
    MONTH("month"),
    YEAR("year"),
    RECENT("recent"),
    RANGE("range")
}

data class DataDurationQueryParams(
    val period: DataTreePeriod? = null,
    val periodArgument: String? = null,
    val year: Int? = null,
    val month: Int? = null,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val reverse: Boolean = false,
    val limit: Int? = null,
    val topN: Int? = null
)

data class DataTreeQueryParams(
    val period: DataTreePeriod,
    val periodArgument: String,
    val level: Int = -1
)

data class TreeNode(
    val name: String,
    val path: String = "",
    val durationSeconds: Long? = null,
    val children: List<TreeNode> = emptyList()
)

data class TreeQueryResult(
    val ok: Boolean,
    val found: Boolean,
    val roots: List<String> = emptyList(),
    val nodes: List<TreeNode> = emptyList(),
    val message: String,
    val operationId: String = "",
    val legacyText: String = "",
    val usesTextFallback: Boolean = false
)

data class DataQueryTextResult(
    val ok: Boolean,
    val outputText: String,
    val message: String,
    val operationId: String = ""
)

data class ReportChartQueryParams(
    val root: String? = null,
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null
)

data class ReportChartPoint(
    val date: String,
    val durationSeconds: Long,
    val epochDay: Long? = null
)

data class ReportChartData(
    val roots: List<String>,
    val selectedRoot: String,
    val lookbackDays: Int,
    val points: List<ReportChartPoint>,
    val averageDurationSeconds: Long? = null,
    val totalDurationSeconds: Long? = null,
    val activeDays: Int? = null,
    val rangeDays: Int? = null,
    val usesLegacyStatsFallback: Boolean = false,
    val schemaVersion: Int? = null,
    val usesSchemaVersionFallback: Boolean = false
)

data class ReportChartQueryResult(
    val ok: Boolean,
    val data: ReportChartData?,
    val message: String,
    val operationId: String = ""
)

data class RuntimeDiagnosticEntry(
    val timestampIso: String,
    val operationId: String,
    val stage: String,
    val ok: Boolean,
    val initialized: Boolean?,
    val message: String,
    val errorLogPath: String = ""
)

data class RuntimeDiagnosticsListResult(
    val ok: Boolean,
    val entries: List<RuntimeDiagnosticEntry>,
    val message: String,
    val diagnosticsLogPath: String = ""
)

data class RuntimeDiagnosticsPayloadResult(
    val ok: Boolean,
    val payload: String,
    val message: String,
    val entryCount: Int = 0,
    val diagnosticsLogPath: String = ""
)
