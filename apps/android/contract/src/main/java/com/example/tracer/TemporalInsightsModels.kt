package com.example.tracer

enum class TemporalSelectionKind(val wireValue: String) {
    SINGLE_DAY("single_day"),
    DATE_RANGE("date_range"),
    RECENT_DAYS("recent_days")
}

enum class InsightsDisplayMode(val wireValue: String) {
    DAY("day"),
    WEEK("week"),
    MONTH("month"),
    YEAR("year"),
    RANGE("range"),
    RECENT("recent")
}

enum class InsightsOperationKind(val wireValue: String) {
    QUERY("query"),
    STRUCTURED_QUERY("structured_query"),
    TARGETS("targets"),
    EXPORT("export")
}

enum class InsightsExportScope(val wireValue: String) {
    SINGLE("single"),
    ALL_MATCHING("all_matching"),
    BATCH_RECENT_LIST("batch_recent_list")
}

enum class InsightsOutputFormat(val wireValue: String) {
    MARKDOWN("markdown"),
    LATEX("latex"),
    TYPST("typst")
}

data class TemporalSelectionPayload(
    val kind: TemporalSelectionKind,
    val date: String? = null,
    val startDate: String? = null,
    val endDate: String? = null,
    val days: Int? = null,
    val anchorDate: String? = null
)

data class TemporalInsightsQueryRequest(
    val displayMode: InsightsDisplayMode,
    val selection: TemporalSelectionPayload,
    val format: InsightsOutputFormat = InsightsOutputFormat.MARKDOWN,
    val locale: String = "en"
)

data class TemporalInsightsTargetsRequest(
    val displayMode: InsightsDisplayMode
)

data class TemporalInsightsExportRequest(
    val displayMode: InsightsDisplayMode,
    val exportScope: InsightsExportScope,
    val format: InsightsOutputFormat = InsightsOutputFormat.MARKDOWN,
    val selection: TemporalSelectionPayload? = null,
    val recentDaysList: List<Int> = emptyList(),
    val locale: String = "en"
)
