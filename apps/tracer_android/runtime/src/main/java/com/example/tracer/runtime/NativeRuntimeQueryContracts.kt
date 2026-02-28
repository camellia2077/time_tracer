package com.example.tracer

internal data class DataQueryRequest(
    val action: Int,
    val outputMode: String? = null,
    val year: Int? = null,
    val month: Int? = null,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val remark: String? = null,
    val dayRemark: String? = null,
    val project: String? = null,
    val root: String? = null,
    val exercise: Int? = null,
    val status: Int? = null,
    val overnight: Boolean = false,
    val reverse: Boolean = false,
    val limit: Int? = null,
    val topN: Int? = null,
    val lookbackDays: Int? = null,
    val scoreByDuration: Boolean = false,
    val treePeriod: String? = null,
    val treePeriodArgument: String? = null,
    val treeMaxDepth: Int? = null
)

internal object DataQueryOutputMode {
    const val TEXT: String = "text"
    const val SEMANTIC_JSON: String = "semantic_json"
}

internal const val REPORT_CHART_SCHEMA_VERSION_V1: Int = 1

internal data class PeriodArgumentValidationResult(
    val argument: String,
    val error: DataQueryTextResult?
)

internal data class ParsedTreeQueryPayload(
    val ok: Boolean,
    val found: Boolean,
    val roots: List<String>,
    val nodes: List<TreeNode>,
    val errorMessage: String
)
