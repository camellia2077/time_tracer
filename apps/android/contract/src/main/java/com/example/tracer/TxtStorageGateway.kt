package com.example.tracer

interface TxtStorageGateway {
    suspend fun inspectTxtFiles(): TxtInspectionResult
    suspend fun listTxtFiles(): TxtHistoryListResult
    suspend fun readTxtFile(relativePath: String): TxtFileContentResult
    suspend fun saveTxtFile(relativePath: String, content: String): TxtFileContentResult =
        TxtFileContentResult(
            ok = false,
            filePath = relativePath,
            content = "",
            message = "TXT save runtime is unavailable."
        )
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

    suspend fun resolveTxtDayEdit(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayEditResolveResult = TxtDayEditResolveResult(
        ok = false,
        normalizedDayMarker = dayMarker,
        found = false,
        isMarkerValid = false,
        canSave = false,
        dayRemark = "",
        events = emptyList(),
        dayContentIsoDate = null,
        message = "TXT structured day-edit runtime is unavailable."
    )

    suspend fun applyTxtDayEdit(
        content: String,
        dayMarker: String,
        selectedMonth: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>
    ): TxtDayEditApplyResult = TxtDayEditApplyResult(
        ok = false,
        normalizedDayMarker = dayMarker,
        found = false,
        isMarkerValid = false,
        updatedContent = content,
        message = "TXT structured day-edit runtime is unavailable."
    )

    suspend fun convertTxtActivityNames(
        content: String,
        direction: TxtActivityNameMappingDirection
    ): TxtActivityNameConversionResult = TxtActivityNameConversionResult(
        ok = false,
        convertedContent = content,
        message = "TXT activity-name conversion runtime is unavailable."
    )

    suspend fun replaceTxtCanonicalActivityNames(
        content: String,
        replacements: List<CanonicalActivityNameReplacement>
    ): TxtCanonicalActivityReplacementResult = TxtCanonicalActivityReplacementResult(
        ok = false,
        updatedContent = content,
        message = "TXT canonical replacement runtime is unavailable."
    )
}
