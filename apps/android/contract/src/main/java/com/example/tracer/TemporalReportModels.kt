package com.example.tracer

enum class TemporalSelectionKind(val wireValue: String) {
    SINGLE_DAY("single_day"),
    DATE_RANGE("date_range"),
    RECENT_DAYS("recent_days")
}

enum class ReportDisplayMode(val wireValue: String) {
    DAY("day"),
    WEEK("week"),
    MONTH("month"),
    YEAR("year"),
    RANGE("range"),
    RECENT("recent")
}

enum class ReportOperationKind(val wireValue: String) {
    QUERY("query"),
    STRUCTURED_QUERY("structured_query"),
    TARGETS("targets"),
    EXPORT("export")
}

enum class ReportExportScope(val wireValue: String) {
    SINGLE("single"),
    ALL_MATCHING("all_matching"),
    BATCH_RECENT_LIST("batch_recent_list")
}

enum class ReportOutputFormat(val wireValue: String) {
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

data class TemporalReportQueryRequest(
    val displayMode: ReportDisplayMode,
    val selection: TemporalSelectionPayload,
    val format: ReportOutputFormat = ReportOutputFormat.MARKDOWN
)

data class TemporalReportTargetsRequest(
    val displayMode: ReportDisplayMode
)

data class TemporalReportExportRequest(
    val displayMode: ReportDisplayMode,
    val exportScope: ReportExportScope,
    val format: ReportOutputFormat = ReportOutputFormat.MARKDOWN,
    val selection: TemporalSelectionPayload? = null,
    val recentDaysList: List<Int> = emptyList()
)
