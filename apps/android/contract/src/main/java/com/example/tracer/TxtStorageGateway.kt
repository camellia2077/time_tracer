package com.example.tracer

interface TxtStorageGateway {
    suspend fun inspectTxtFiles(): TxtInspectionResult
    suspend fun listTxtFiles(): TxtHistoryListResult
    suspend fun readTxtFile(relativePath: String): TxtFileContentResult
    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult

    suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult = TxtDayMarkerResult(
        ok = false,
        normalizedDayMarker = "",
        message = "TXT day-block runtime is unavailable."
    )

    suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = TxtDayBlockResolveResult(
        ok = false,
        normalizedDayMarker = dayMarker,
        found = false,
        isMarkerValid = false,
        canSave = false,
        dayBody = "",
        dayContentIsoDate = null,
        message = "TXT day-block runtime is unavailable."
    )

    suspend fun replaceTxtDayBlock(
        content: String,
        dayMarker: String,
        editedDayBody: String
    ): TxtDayBlockReplaceResult = TxtDayBlockReplaceResult(
        ok = false,
        normalizedDayMarker = dayMarker,
        found = false,
        isMarkerValid = false,
        updatedContent = content,
        message = "TXT day-block runtime is unavailable."
    )
}
