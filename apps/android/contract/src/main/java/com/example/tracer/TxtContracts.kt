package com.example.tracer

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
