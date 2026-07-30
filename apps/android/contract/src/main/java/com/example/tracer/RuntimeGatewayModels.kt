package com.example.tracer

data class NativeCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = ""
)

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

data class ActivityTimelineItem(
    val logicalId: Long = 0L,
    val startTime: String,
    val endTime: String,
    val activityName: String,
    val durationSeconds: Long,
    val remark: String? = null
)

data class StructuredDailyReport(
    val date: String,
    val totalDurationSeconds: Long,
    val dayRemark: String = "",
    val activities: List<ActivityTimelineItem> = emptyList()
)

data class StructuredReportCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val report: StructuredDailyReport?,
    val rawResponse: String,
    val errorMessage: String = "",
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

data class ClearDatabaseResult(
    val ok: Boolean,
    val message: String
)

data class DataFolderSnapshotResult(
    val ok: Boolean,
    val message: String,
    val txtFileCount: Int = 0,
    val tomlFileCount: Int = 0
)

data class RecordActionResult(
    val ok: Boolean,
    val message: String,
    val operationId: String = ""
)

data class TracerExchangePayloadItem(
    val relativePathHint: String,
    val content: String
)

data class TracerExchangeExportResult(
    val ok: Boolean,
    val message: String,
    val outputPath: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val converterFileCount: Int = 0,
    val manifestIncluded: Boolean = false
)

data class TracerExchangeImportResult(
    val ok: Boolean,
    val message: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val replacedMonthCount: Int = 0,
    val preservedMonthCount: Int = 0,
    val rebuiltMonthCount: Int = 0,
    val textRootUpdated: Boolean = false,
    val configApplied: Boolean = false,
    val databaseRebuilt: Boolean = false,
    val retainedFailureRoot: String = "",
    val backupRetainedRoot: String = "",
    val backupCleanupError: String = ""
)

data class TracerExchangeInspectResult(
    val ok: Boolean,
    val message: String,
    val renderedText: String,
    val inputPath: String,
    val sourceRootName: String,
    val payloadFileCount: Int,
    val packageVersion: Int,
    val producerPlatform: String,
    val producerApp: String,
    val createdAtUtc: String
)

data class TxtHistoryListResult(
    val ok: Boolean,
    val files: List<String>,
    val message: String
)

enum class TxtSyncState {
    SYNCED,
    NOT_INGESTED,
    HEADER_INVALID,
    PATH_MISMATCH,
    DB_HASH_MISMATCH,
    DB_PATH_MISMATCH,
    DUPLICATE_MONTH
}

data class TxtInspectionEntry(
    val relativePath: String,
    val headerMonth: String?,
    val expectedCanonicalRelativePath: String?,
    val syncState: TxtSyncState,
    val canOpen: Boolean,
    val message: String
)

data class TxtInspectionResult(
    val ok: Boolean,
    val entries: List<TxtInspectionEntry>,
    val message: String
)

data class TxtFileContentResult(
    val ok: Boolean,
    val filePath: String,
    val content: String,
    val message: String
)

data class TxtDayMarkerResult(
    val ok: Boolean,
    val normalizedDayMarker: String,
    val message: String
)

data class TxtDayBlockResolveResult(
    val ok: Boolean,
    val normalizedDayMarker: String,
    val found: Boolean,
    val isMarkerValid: Boolean,
    val canSave: Boolean,
    val dayBody: String,
    val dayContentIsoDate: String?,
    val message: String
)

data class TxtDayBlockReplaceResult(
    val ok: Boolean,
    val normalizedDayMarker: String,
    val found: Boolean,
    val isMarkerValid: Boolean,
    val updatedContent: String,
    val message: String
)

enum class TxtActivityNameMappingDirection(val wireValue: String) {
    ALIAS_TO_CANONICAL("alias_to_canonical"),
    CANONICAL_TO_ALIAS("canonical_to_alias")
}

data class TxtActivityNameConversionResult(
    val ok: Boolean,
    val convertedContent: String,
    val message: String
)

data class CanonicalActivityNameReplacement(
    val oldCanonical: String,
    val newCanonical: String
)

data class TxtCanonicalActivityReplacementResult(
    val ok: Boolean,
    val updatedContent: String,
    val message: String
)

enum class ActivityHierarchyNodeKind(val wireValue: String) {
    LEAF("leaf"),
    GROUP("group");

    companion object {
        fun fromWireValue(value: String, legacyIsGroup: Boolean): ActivityHierarchyNodeKind =
            entries.firstOrNull { it.wireValue == value } ?: if (legacyIsGroup) GROUP else LEAF
    }
}

data class ActivityHierarchyNode(
    val canonicalKey: String,
    val path: String,
    val kind: ActivityHierarchyNodeKind,
    val aliases: List<String>,
    val children: List<ActivityHierarchyNode>
) {
    /** Compatibility view for the existing presentation adapter. */
    val isGroup: Boolean
        get() = kind == ActivityHierarchyNodeKind.GROUP
}

data class ActivityHierarchySnapshot(
    val parent: String,
    val nodes: List<ActivityHierarchyNode>
)

data class ActivityHierarchyDescribeResult(
    val ok: Boolean,
    val hierarchy: ActivityHierarchySnapshot? = null,
    val message: String = ""
)

data class ActivityHierarchyDocumentInput(
    val sourceName: String,
    val tomlContent: String
)

data class ActivityHierarchyValidationResult(
    val ok: Boolean,
    val message: String = ""
)

enum class ActivityHierarchyOperationKind(val wireValue: String) {
    ADD_GROUP("add_group"),
    DELETE_GROUP("delete_group"),
    RENAME_GROUP_CANONICAL("rename_group_canonical"),
    SET_GROUP_ALIASES("set_group_aliases"),
    RENAME_GROUP_ALIAS("rename_group_alias"),
    APPEND_GROUP_ALIAS("append_group_alias"),
    MOVE_GROUP("move_group"),
    ADD_LEAF("add_leaf"),
    DELETE_LEAF("delete_leaf"),
    RENAME_LEAF_CANONICAL("rename_leaf_canonical"),
    SET_LEAF_ALIASES("set_leaf_aliases"),
    APPEND_LEAF_ALIAS("append_leaf_alias"),
    MOVE_LEAF("move_leaf"),
    PROMOTE_LEAF("promote_leaf"),
    RENAME_PARENT("rename_parent")
}

data class ActivityHierarchyOperation(
    val kind: ActivityHierarchyOperationKind,
    val targetPath: String = "",
    val destinationPath: String = "",
    val canonicalKey: String = "",
    val newName: String = "",
    val oldParent: String = "",
    val targetAlias: String = "",
    val oldAlias: String = "",
    val aliases: List<String> = emptyList()
)

data class AliasKeyReplacement(
    val oldAlias: String,
    val newAlias: String
)

/**
 * One Core-produced replacement plan. Canonical and alias entries share the
 * same old-token -> new-token meaning; the two lists only preserve the token
 * namespace required by the corresponding Core TXT action.
 */
data class ActivityNameReplacementPlan(
    val canonical: List<CanonicalActivityNameReplacement> = emptyList(),
    val aliases: List<AliasKeyReplacement> = emptyList()
) {
    val isEmpty: Boolean
        get() = canonical.isEmpty() && aliases.isEmpty()

    companion object {
        fun fromCore(
            canonical: List<CanonicalActivityNameReplacement>,
            aliases: List<AliasKeyReplacement>
        ): ActivityNameReplacementPlan = ActivityNameReplacementPlan(
            canonical = canonical,
            aliases = aliases
        )
    }
}

data class ActivityHierarchyOperationResult(
    val ok: Boolean,
    val updatedTomlContent: String,
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val hierarchy: ActivityHierarchySnapshot? = null,
    val message: String = ""
)

data class ActivityHierarchyDocumentOutput(
    val sourceName: String,
    val updatedTomlContent: String
)

data class ActivityHierarchyDocumentRename(
    val oldSourceName: String,
    val newSourceName: String
)

data class ActivityHierarchyCrossDocumentOperationResult(
    val ok: Boolean,
    val updatedDocuments: List<ActivityHierarchyDocumentOutput> = emptyList(),
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val message: String = ""
)

data class ActivityHierarchyMigrationRequest(
    val configRelativePath: String,
    val updatedTomlContent: String,
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val updatedDocuments: List<ActivityHierarchyDocumentInput> = emptyList(),
    val configFileRename: ActivityHierarchyDocumentRename? = null,
    val allowMissingConfig: Boolean = false
)

data class ActivityHierarchyMigrationResult(
    val ok: Boolean,
    val message: String,
    val updatedTxtFileCount: Int = 0,
    val updatedTomlContent: String = "",
    val updatedConfigRelativePath: String = ""
)

data class ConfigTomlFileEntry(
    val relativePath: String,
    val displayName: String
)

data class ConfigTomlListResult(
    val ok: Boolean,
    val aliasFiles: List<ConfigTomlFileEntry>,
    val chartFiles: List<ConfigTomlFileEntry>,
    val metaFiles: List<ConfigTomlFileEntry>,
    val reportFiles: List<ConfigTomlFileEntry>,
    val message: String,
    /** All mutable TOML files under the canonical config/user root. */
    val userFiles: List<ConfigTomlFileEntry> = emptyList()
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

data class ActivityAliasMappingEntry(
    val alias: String,
    val canonical: String
)

data class ActivityAliasMappingListResult(
    val ok: Boolean,
    val entries: List<ActivityAliasMappingEntry>,
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

data class ReportCalendarAvailabilityResult(
    val ok: Boolean,
    val years: List<String> = emptyList(),
    val months: List<String> = emptyList(),
    val message: String = "",
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

data class ReportCompositionQueryParams(
    val lookbackDays: Int = 7,
    val fromDateIso: String? = null,
    val toDateIso: String? = null
)

data class ReportCompositionSlice(
    val root: String,
    val durationSeconds: Long,
    val percent: Float
)

data class ReportCompositionData(
    val totalDurationSeconds: Long,
    val activeRootCount: Int,
    val rangeDays: Int,
    // The weighted activity tree is the sole composition representation.
    val tree: List<TreeNode>
)

data class ReportCompositionQueryResult(
    val ok: Boolean,
    val data: ReportCompositionData?,
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
