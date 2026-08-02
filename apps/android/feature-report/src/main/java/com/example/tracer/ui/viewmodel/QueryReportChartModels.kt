package com.example.tracer

internal data class DomainChartPoint(
    val date: String,
    val durationSeconds: Long,
    val epochDay: Long? = null
)

internal data class DomainChartModel(
    val roots: List<String>,
    val selectedRoot: String,
    val lookbackDays: Int,
    val points: List<DomainChartPoint>,
    val averageDurationSeconds: Long,
    val totalDurationSeconds: Long,
    val activeDays: Int,
    val rangeDays: Int,
    val usesLegacyStatsFallback: Boolean,
    val schemaVersion: Int?,
    val usesSchemaVersionFallback: Boolean,
)

data class ChartRenderModel(
    val roots: List<String>,
    val selectedRoot: String,
    val lookbackDays: Int,
    val points: List<ReportChartPoint>,
    val averageDurationSeconds: Long,
    val totalDurationSeconds: Long,
    val activeDays: Int,
    val rangeDays: Int,
    val usesLegacyStatsFallback: Boolean,
    val schemaVersion: Int?,
    val usesSchemaVersionFallback: Boolean,
    val fromDateIso: String? = null,
    val toDateIso: String? = null
)

data class CompositionChartRenderModel(
    val totalDurationSeconds: Long,
    val activeRootCount: Int,
    val activeDays: Int = 0,
    val rangeDays: Int,
    val averageDayBasis: ReportAverageDayBasis = ReportAverageDayBasis.ACTIVE_DAYS,
    val averageDenominatorDays: Int = 0,
    val displayLevel: Int = 0,
    val displayPath: List<String> = emptyList(),
    val tree: List<TreeNode>
)

data class ChartQueryTrace(
    val operationId: String,
    val parameterHash: String,
    val durationMs: Long,
    val pointCount: Int,
    val rootCount: Int,
    val cacheHit: Boolean
)
