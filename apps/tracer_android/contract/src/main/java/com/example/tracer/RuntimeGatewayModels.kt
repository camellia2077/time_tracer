package com.example.tracer

data class NativeCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val rawResponse: String
)

data class ReportCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val outputText: String,
    val rawResponse: String
)

data class ClearAndInitResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val clearMessage: String,
    val initResponse: String
)

data class ClearTxtResult(
    val ok: Boolean,
    val message: String
)

data class RecordActionResult(
    val ok: Boolean,
    val message: String
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
    val message: String
)

data class ActivityMappingNamesResult(
    val ok: Boolean,
    val names: List<String>,
    val message: String
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

data class DataQueryTextResult(
    val ok: Boolean,
    val outputText: String,
    val message: String
)
