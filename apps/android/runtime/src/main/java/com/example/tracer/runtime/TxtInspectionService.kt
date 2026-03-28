package com.example.tracer

internal class TxtInspectionService(
    private val ensureTextStorage: () -> TextStorage,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeListTxtIngestSyncStatus: (String) -> String,
    private val responseCodec: NativeResponseCodec,
    private val statusCodec: NativeIngestSyncStatusCodec
) {
    fun inspectTxtFiles(): TxtInspectionResult {
        val storage = ensureTextStorage()
        val listResult = storage.listTxtFiles()
        if (!listResult.ok) {
            return TxtInspectionResult(
                ok = false,
                entries = emptyList(),
                message = listResult.message
            )
        }

        val draftEntries = mutableListOf<DraftTxtInspectionEntry>()
        for (path in listResult.files.sorted()) {
            val readResult = storage.readTxtFile(path)
            if (!readResult.ok) {
                return TxtInspectionResult(
                    ok = false,
                    entries = emptyList(),
                    message = readResult.message
                )
            }

            val header = parseTxtMonthHeader(readResult.content)
            draftEntries += DraftTxtInspectionEntry(
                relativePath = readResult.filePath.replace('\\', '/'),
                header = header,
                contentHashSha256 = header?.let {
                    computeCanonicalTxtSha256Hex(readResult.content)
                }
            )
        }

        val distinctMonths = draftEntries
            .mapNotNull { it.header?.monthKey }
            .distinct()
            .sorted()
        val dbStatuses = queryNativeSyncStatuses(distinctMonths)
        if (!dbStatuses.ok) {
            return TxtInspectionResult(
                ok = false,
                entries = emptyList(),
                message = "inspect txt failed: ${dbStatuses.errorMessage}"
            )
        }

        val statusByMonth = dbStatuses.items.associateBy { it.monthKey }
        val duplicateMonths = draftEntries
            .mapNotNull { it.header?.monthKey }
            .groupingBy { it }
            .eachCount()
            .filterValues { it > 1 }
            .keys

        val entries = draftEntries.map { entry ->
            buildInspectionEntry(
                entry = entry,
                duplicateMonths = duplicateMonths,
                statusByMonth = statusByMonth
            )
        }

        return TxtInspectionResult(
            ok = true,
            entries = entries,
            message = "Inspected ${entries.size} TXT file(s)."
        )
    }

    private fun queryNativeSyncStatuses(months: List<String>): NativeTxtIngestSyncStatusResult {
        if (months.isEmpty()) {
            return NativeTxtIngestSyncStatusResult(
                ok = true,
                items = emptyList(),
                errorMessage = ""
            )
        }

        val requestJson = statusCodec.buildRequest(months)
        val result = executeAfterInit("native_list_txt_ingest_sync_status") {
            nativeListTxtIngestSyncStatus(requestJson)
        }
        if (!result.initialized || !result.operationOk) {
            val payload = responseCodec.parse(result.rawResponse)
            return NativeTxtIngestSyncStatusResult(
                ok = false,
                items = emptyList(),
                errorMessage = payload.errorMessage.ifEmpty { result.rawResponse }
            )
        }

        val parsed = statusCodec.parse(result.rawResponse)
        return parsed
    }

    private fun buildInspectionEntry(
        entry: DraftTxtInspectionEntry,
        duplicateMonths: Set<String>,
        statusByMonth: Map<String, NativeTxtIngestSyncStatusEntry>
    ): TxtInspectionEntry {
        val header = entry.header
        val headerMonth = header?.monthKey
        val expectedPath = header?.canonicalRelativePath
        val dbEntry = headerMonth?.let(statusByMonth::get)

        if (header == null) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = null,
                expectedCanonicalRelativePath = null,
                syncState = TxtSyncState.HEADER_INVALID,
                canOpen = false,
                message = "TXT is missing valid yYYYY + mMM headers."
            )
        }

        if (duplicateMonths.contains(headerMonth)) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = headerMonth,
                expectedCanonicalRelativePath = expectedPath,
                syncState = TxtSyncState.DUPLICATE_MONTH,
                canOpen = false,
                message = "Multiple TXT files resolve to $headerMonth. Remove duplicates before opening."
            )
        }

        if (entry.relativePath != expectedPath) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = headerMonth,
                expectedCanonicalRelativePath = expectedPath,
                syncState = TxtSyncState.PATH_MISMATCH,
                canOpen = false,
                message = "TXT header month resolves to $expectedPath, but file is stored at ${entry.relativePath}."
            )
        }

        if (dbEntry == null) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = headerMonth,
                expectedCanonicalRelativePath = expectedPath,
                syncState = TxtSyncState.NOT_INGESTED,
                canOpen = true,
                message = "TXT header/path are valid, but this version has not been ingested into DB yet."
            )
        }

        if (dbEntry.txtRelativePath != entry.relativePath) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = headerMonth,
                expectedCanonicalRelativePath = expectedPath,
                syncState = TxtSyncState.DB_PATH_MISMATCH,
                canOpen = false,
                message = "DB sync state points to ${dbEntry.txtRelativePath}, but local TXT is ${entry.relativePath}."
            )
        }

        if (dbEntry.txtContentHashSha256 != entry.contentHashSha256) {
            return TxtInspectionEntry(
                relativePath = entry.relativePath,
                headerMonth = headerMonth,
                expectedCanonicalRelativePath = expectedPath,
                syncState = TxtSyncState.DB_HASH_MISMATCH,
                canOpen = false,
                message = "TXT content differs from the version last ingested into DB."
            )
        }

        return TxtInspectionEntry(
            relativePath = entry.relativePath,
            headerMonth = headerMonth,
            expectedCanonicalRelativePath = expectedPath,
            syncState = TxtSyncState.SYNCED,
            canOpen = true,
            message = "TXT header/path/hash match the current DB sync state."
        )
    }

    private data class DraftTxtInspectionEntry(
        val relativePath: String,
        val header: TxtMonthHeader?,
        val contentHashSha256: String?
    )
}
