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
    val rootTree: List<TreeNode>,
    val averageDurationSeconds: Long,
    val totalOccurrenceCount: Long,
    val averageDurationPerOccurrenceSeconds: Long,
    val modeDurationSeconds: Double?,
    val medianDurationSeconds: Double?,
    val minimumDurationSeconds: Double?,
    val maximumDurationSeconds: Double?,
    val lowerQuartileDurationSeconds: Double?,
    val upperQuartileDurationSeconds: Double?,
    val coefficientOfVariation: Double?,
    val meanAbsoluteDeviationSeconds: Double?,
    val totalDurationSeconds: Long,
    val activeDays: Int,
    val rangeDays: Int,
    val schemaVersion: Int?,
    val usesSchemaVersionFallback: Boolean,
)

data class ChartRenderModel(
    val roots: List<String>,
    val selectedRoot: String,
    val lookbackDays: Int,
    val points: List<InsightsChartPoint>,
    val rootTree: List<TreeNode> = emptyList(),
    val averageDurationSeconds: Long,
    val totalOccurrenceCount: Long,
    val averageDurationPerOccurrenceSeconds: Long,
    val modeDurationSeconds: Double?,
    val medianDurationSeconds: Double?,
    val minimumDurationSeconds: Double?,
    val maximumDurationSeconds: Double?,
    val lowerQuartileDurationSeconds: Double?,
    val upperQuartileDurationSeconds: Double?,
    val coefficientOfVariation: Double?,
    val meanAbsoluteDeviationSeconds: Double?,
    val totalDurationSeconds: Long,
    val activeDays: Int,
    val rangeDays: Int,
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
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS,
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

internal data class ChartComparisonResult(
    val renderModel: ChartRenderModel?,
    val errorMessage: String = ""
)
