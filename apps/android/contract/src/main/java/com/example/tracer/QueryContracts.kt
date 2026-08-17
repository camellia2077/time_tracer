package com.example.tracer

data class ActivityFrequentResult(
    val ok: Boolean,
    val frequentActivities: List<String>,
    val message: String,
    val operationId: String = ""
)

data class ActivityMappingNamesResult(
    val ok: Boolean,
    val names: List<String>,
    val message: String,
    val operationId: String = ""
)

data class ActivityHierarchyLeafMappingEntry(
    val alias: String,
    val canonical: String
)

data class ActivityHierarchyLeafMappingListResult(
    val ok: Boolean,
    val entries: List<ActivityHierarchyLeafMappingEntry>,
    val message: String,
    val operationId: String = ""
)

data class CanonicalCatalogEntry(
    val canonicalLeaf: String,
    val canonicalPath: String,
    val sourceFilePath: String,
    val aliases: List<String> = emptyList()
)

data class CanonicalPathNode(
    val name: String,
    val path: String,
    val entries: List<CanonicalCatalogEntry> = emptyList(),
    val children: List<CanonicalPathNode> = emptyList()
)

data class CanonicalCatalogResult(
    val ok: Boolean,
    val roots: List<CanonicalPathNode>,
    val entries: List<CanonicalCatalogEntry>,
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

data class TreeNode(
    val name: String,
    val path: String = "",
    val durationSeconds: Long? = null,
    val occurrenceCount: Long? = null,
    val averageDurationSeconds: Long? = null,
    val averageDurationPerOccurrenceSeconds: Long? = null,
    val averageOccurrenceCount: Double? = null,
    val averageOccurrenceRatio: Double? = null,
    val parentDurationPercent: Float? = null,
    val children: List<TreeNode> = emptyList()
)

data class TreeQueryResult(
    val ok: Boolean,
    val found: Boolean,
    val roots: List<String> = emptyList(),
    val nodes: List<TreeNode> = emptyList(),
    val message: String,
    val operationId: String = "",
    val maxAvailableDepth: Int = 0
)

data class DataQueryTextResult(
    val ok: Boolean,
    val outputText: String,
    val message: String,
    val operationId: String = ""
)

data class PreviousActivityTail(
    val dateIso: String,
    val endTime: String
)

data class PreviousActivityTailResult(
    val ok: Boolean,
    val found: Boolean,
    val tail: PreviousActivityTail? = null,
    val message: String,
    val operationId: String = ""
)

data class LatestActivityRecord(
    val dateIso: String,
    val activity: String,
    val recordKind: String,
    val startTime: String,
    val endTime: String,
    val durationSeconds: Int
)

data class LatestActivityRecordResult(
    val ok: Boolean,
    val found: Boolean,
    val record: LatestActivityRecord? = null,
    val message: String,
    val operationId: String = ""
)
