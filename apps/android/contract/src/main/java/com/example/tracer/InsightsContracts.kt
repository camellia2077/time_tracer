package com.example.tracer

data class InsightsErrorContract(
    val errorCode: String,
    val errorCategory: String,
    val hints: List<String> = emptyList()
)

data class InsightsWindowMetadata(
    val hasRecords: Boolean,
    val matchedDayCount: Int,
    val matchedRecordCount: Int,
    val startDate: String,
    val endDate: String,
    val requestedDays: Int
)

data class InsightsCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val outputText: String,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = "",
    val errorContract: InsightsErrorContract? = null,
    val insightsWindowMetadata: InsightsWindowMetadata? = null
)

enum class ActivityTimelineRecordKind(val wireValue: String) {
    INTERVAL("interval"),
    END_ONLY("end_only");

    companion object {
        fun fromWireValue(value: String): ActivityTimelineRecordKind =
            entries.firstOrNull { it.wireValue == value }
                ?: error("Unknown structured insights record_kind: $value")
    }
}

data class ActivityTimelineItem(
    val logicalId: Long = 0L,
    val startTime: String,
    val endTime: String,
    val activityName: String,
    val durationSeconds: Long,
    val remark: String? = null,
    val kind: ActivityTimelineRecordKind = ActivityTimelineRecordKind.INTERVAL
)

data class StructuredDailyInsights(
    val date: String,
    val totalDurationSeconds: Long,
    val dayRemark: String = "",
    val statuses: List<InsightsStatusValue> = emptyList(),
    val activities: List<ActivityTimelineItem> = emptyList()
)

data class InsightsStatusValue(
    val id: String,
    val label: String,
    val occurrenceCount: Int,
    val totalDurationSeconds: Long
)

data class StructuredInsightsProjectNode(
    val name: String,
    val durationSeconds: Long,
    val children: List<StructuredInsightsProjectNode> = emptyList()
)

data class StructuredInsightsCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val insights: StructuredDailyInsights?,
    val rawResponse: String,
    val activityDays: List<StructuredDailyInsights> = emptyList(),
    val projectTree: List<StructuredInsightsProjectNode> = emptyList(),
    val statuses: List<InsightsStatusValue> = emptyList(),
    val errorMessage: String = "",
    val operationId: String = ""
)

data class InsightsCalendarAvailabilityResult(
    val ok: Boolean,
    val years: List<String> = emptyList(),
    val months: List<String> = emptyList(),
    val message: String = "",
    val operationId: String = ""
)

enum class InsightsAverageDayBasis(val wireValue: String) {
    ACTIVE_DAYS("active_days"),
    CALENDAR_DAYS("calendar_days")
}

data class InsightsChartQueryParams(
    val root: String? = null,
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS
)

data class InsightsChartPoint(
    val date: String,
    val durationSeconds: Long,
    val epochDay: Long? = null
)

data class InsightsChartData(
    val roots: List<String>,
    val selectedRoot: String,
    val lookbackDays: Int,
    val points: List<InsightsChartPoint>,
    val averageDurationSeconds: Long? = null,
    val totalDurationSeconds: Long? = null,
    val activeDays: Int? = null,
    val rangeDays: Int? = null,
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS,
    val averageDenominatorDays: Int? = null,
    val usesLegacyStatsFallback: Boolean = false,
    val schemaVersion: Int? = null,
    val usesSchemaVersionFallback: Boolean = false
)

data class InsightsChartQueryResult(
    val ok: Boolean,
    val data: InsightsChartData?,
    val message: String,
    val operationId: String = ""
)

data class InsightsCompositionQueryParams(
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS
)

data class InsightsCompositionSlice(
    val root: String,
    val durationSeconds: Long,
    val percent: Float,
    val totalDurationSeconds: Long? = null,
    val occurrenceCount: Long? = null,
    val averageDurationSeconds: Long? = null,
    val averageOccurrenceCount: Double? = null,
    val averageOccurrenceRatio: Double? = null
)

data class InsightsCompositionData(
    val totalDurationSeconds: Long,
    val activeRootCount: Int,
    val activeDays: Int = 0,
    val rangeDays: Int,
    val averageDayBasis: InsightsAverageDayBasis = InsightsAverageDayBasis.ACTIVE_DAYS,
    val averageDenominatorDays: Int = 0,
    val displayLevel: Int = 0,
    val displayPath: List<String> = emptyList(),
    // The weighted activity tree is the sole composition representation.
    val tree: List<TreeNode>
)

data class InsightsCompositionQueryResult(
    val ok: Boolean,
    val data: InsightsCompositionData?,
    val message: String,
    val operationId: String = ""
)
