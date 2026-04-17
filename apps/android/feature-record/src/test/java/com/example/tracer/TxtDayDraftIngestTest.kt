package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class TxtDayDraftIngestTest {
    @Test
    fun ingestDayDraft_mergesDayBody_andThenSavesMonth() = runBlocking {
        val gateway = FakeTxtDayDraftGateway(
            replaceResult = TxtDayBlockReplaceResult(
                ok = true,
                normalizedDayMarker = "0417",
                found = true,
                isMarkerValid = true,
                updatedContent = "y2026\nm04\n0417\nnew-body\n",
                message = "ok"
            )
        )
        var mergedMonthContent = ""
        var saveCalled = false

        val result = ingestDayDraft(
            txtStorageGateway = gateway,
            monthContent = "y2026\nm04\n0417\nold-body\n",
            dayMarker = "0417",
            dayDraftBody = "new-body\n",
            onMergedMonthContent = { mergedMonthContent = it },
            onSaveHistoryFile = { saveCalled = true }
        )

        assertTrue(result)
        assertEquals("y2026\nm04\n0417\nold-body\n", gateway.lastReplaceContent)
        assertEquals("0417", gateway.lastReplaceDayMarker)
        assertEquals("new-body\n", gateway.lastReplaceDayBody)
        assertEquals("y2026\nm04\n0417\nnew-body\n", mergedMonthContent)
        assertTrue(saveCalled)
    }

    @Test
    fun ingestDayDraft_replaceFailure_doesNotMutateMonthOrSave() = runBlocking {
        val gateway = FakeTxtDayDraftGateway(
            replaceResult = TxtDayBlockReplaceResult(
                ok = false,
                normalizedDayMarker = "0417",
                found = true,
                isMarkerValid = true,
                updatedContent = "ignored",
                message = "failed"
            )
        )
        var mergedMonthContent = ""
        var saveCalled = false

        val result = ingestDayDraft(
            txtStorageGateway = gateway,
            monthContent = "y2026\nm04\n0417\nold-body\n",
            dayMarker = "0417",
            dayDraftBody = "new-body\n",
            onMergedMonthContent = { mergedMonthContent = it },
            onSaveHistoryFile = { saveCalled = true }
        )

        assertFalse(result)
        assertEquals("", mergedMonthContent)
        assertFalse(saveCalled)
    }
}

private class FakeTxtDayDraftGateway(
    private val replaceResult: TxtDayBlockReplaceResult
) : TxtStorageGateway {
    var lastReplaceContent: String = ""
        private set
    var lastReplaceDayMarker: String = ""
        private set
    var lastReplaceDayBody: String = ""
        private set

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = emptyList(),
        message = "ok"
    )

    override suspend fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = emptyList(),
        message = "ok"
    )

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult = TxtFileContentResult(
        ok = true,
        filePath = relativePath,
        content = "",
        message = "ok"
    )

    override suspend fun saveTxtFileAndSync(
        relativePath: String,
        content: String
    ): RecordActionResult = RecordActionResult(ok = true, message = "ok")

    override suspend fun replaceTxtDayBlock(
        content: String,
        dayMarker: String,
        editedDayBody: String
    ): TxtDayBlockReplaceResult {
        lastReplaceContent = content
        lastReplaceDayMarker = dayMarker
        lastReplaceDayBody = editedDayBody
        return replaceResult
    }
}
