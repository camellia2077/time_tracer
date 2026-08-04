package com.example.tracer

data class ReportErrorContract(
    val errorCode: String,
    val errorCategory: String,
    val hints: List<String> = emptyList()
)

data class ReportWindowMetadata(
    val hasRecords: Boolean,
    val matchedDayCount: Int,
    val matchedRecordCount: Int,
    val startDate: String,
    val endDate: String,
    val requestedDays: Int
)

data class ReportCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val outputText: String,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = "",
    val errorContract: ReportErrorContract? = null,
    val reportWindowMetadata: ReportWindowMetadata? = null
)

enum class ActivityTimelineRecordKind(val wireValue: String) {
    INTERVAL("interval"),
    END_ONLY("end_only");

    companion object {
        fun fromWireValue(value: String): ActivityTimelineRecordKind =
            entries.firstOrNull { it.wireValue == value }
                ?: error("Unknown structured report record_kind: $value")
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

data class StructuredDailyReport(
    val date: String,
    val totalDurationSeconds: Long,
    val dayRemark: String = "",
    val statuses: List<DailyStatusValue> = emptyList(),
    val activities: List<ActivityTimelineItem> = emptyList()
)

data class DailyStatusValue(
    val id: String,
    val label: String,
    val value: Boolean
)

data class StructuredReportCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val report: StructuredDailyReport?,
    val rawResponse: String,
    val errorMessage: String = "",
    val operationId: String = ""
)

data class ReportCalendarAvailabilityResult(
    val ok: Boolean,
    val years: List<String> = emptyList(),
    val months: List<String> = emptyList(),
    val message: String = "",
    val operationId: String = ""
)

enum class ReportAverageDayBasis(val wireValue: String) {
    ACTIVE_DAYS("active_days"),
    CALENDAR_DAYS("calendar_days")
}

data class ReportChartQueryParams(
    val root: String? = null,
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val averageDayBasis: ReportAverageDayBasis = ReportAverageDayBasis.ACTIVE_DAYS
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
    val averageDayBasis: ReportAverageDayBasis = ReportAverageDayBasis.ACTIVE_DAYS,
    val averageDenominatorDays: Int? = null,
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

data class ReportCompositionQueryParams(
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null,
    val averageDayBasis: ReportAverageDayBasis = ReportAverageDayBasis.ACTIVE_DAYS
)

data class ReportCompositionSlice(
    val root: String,
    val durationSeconds: Long,
    val percent: Float,
    val totalDurationSeconds: Long? = null,
    val occurrenceCount: Long? = null,
    val averageDurationSeconds: Long? = null,
    val averageOccurrenceCount: Double? = null,
    val averageOccurrenceRatio: Double? = null
)

data class ReportCompositionData(
    val totalDurationSeconds: Long,
    val activeRootCount: Int,
    val activeDays: Int = 0,
    val rangeDays: Int,
    val averageDayBasis: ReportAverageDayBasis = ReportAverageDayBasis.ACTIVE_DAYS,
    val averageDenominatorDays: Int = 0,
    val displayLevel: Int = 0,
    val displayPath: List<String> = emptyList(),
    // The weighted activity tree is the sole composition representation.
    val tree: List<TreeNode>
)

data class ReportCompositionQueryResult(
    val ok: Boolean,
    val data: ReportCompositionData?,
    val message: String,
    val operationId: String = ""
)
