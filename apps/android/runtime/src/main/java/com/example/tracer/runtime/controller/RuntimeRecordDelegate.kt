package com.example.tracer

import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

private const val REMARK_UPDATE_LOG_TAG = "TimeTracerRemarkUpdate"
private const val RECORD_LOG_TAG = "TimeTracerRecord"

internal class RuntimeRecordDelegate(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val rawRecordStore: InputRecordStore,
    private val loadWakeKeywords: suspend () -> ActivityMappingNamesResult,
    private val ensureActivityHierarchyEntry: suspend (String) -> ActivityHierarchyAutoRegistrationResult = {
        ActivityHierarchyAutoRegistrationResult(ok = true)
    },
    private val defaultTxtDayMarker: suspend (selectedMonth: String, targetDateIso: String) -> TxtDayMarkerResult,
    private val resolveTxtDayBlock: suspend (
        content: String,
        dayMarker: String,
        selectedMonth: String
    ) -> TxtDayBlockResolveResult,
    private val replaceTxtDayBlock: suspend (
        content: String,
        dayMarker: String,
        editedDayBody: String
    ) -> TxtDayBlockReplaceResult,
    private val responseCodec: NativeResponseCodec,
    private val atomicRecordCodec: NativeAtomicRecordCodec,
    private val recordTranslator: NativeRecordTranslator,
    private val inspectTxtFilesInternal: () -> TxtInspectionResult,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeValidateStructure: (inputPath: String) -> String,
    private val nativeValidateLogic: (inputPath: String, dateCheckMode: Int) -> String,
    private val nativeRecordActivityAtomically: (
        targetDateIso: String,
        rawActivityName: String,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int,
        timeOrderMode: RecordTimeOrderMode
    ) -> String,
    private val nativeUpdateActivityRemarkAtomically: (
        targetDateIso: String,
        logicalId: Long,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int
    ) -> String = { _, _, _, _, _ ->
        """{"ok":false,"error_message":"activity remark update is not wired."}"""
    },
    private val nativeUpdateDayRemarkAtomically: (
        targetDateIso: String,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int
    ) -> String = { _, _, _, _ ->
        """{"ok":false,"error_message":"day remark update is not wired."}"""
    },
    private val nativeIngestSingleTxtReplaceMonth: (
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ) -> String,
    private val nativeClearTxtIngestSyncStatus: () -> String
) {
    private val atomicFlow = RuntimeRecordAtomicFlow(
        responseCodec = responseCodec,
        atomicRecordCodec = atomicRecordCodec,
        executeAfterInit = executeAfterInit,
        nativeRecordActivityAtomically = nativeRecordActivityAtomically
    )
    private val syncFlow = RuntimeRecordSyncFlow(
        ensureRuntimePaths = ensureRuntimePaths,
        inspectTxtFilesInternal = inspectTxtFilesInternal,
        executeAfterInit = executeAfterInit,
        nativeIngestSingleTxtReplaceMonth = nativeIngestSingleTxtReplaceMonth,
        nativeClearTxtIngestSyncStatus = nativeClearTxtIngestSyncStatus
    )
    private val txtSaveAndSyncFlow = RuntimeTxtSaveAndSyncFlow(
        ensureRuntimePaths = ensureRuntimePaths,
        ensureTextStorage = ensureTextStorage,
        rawRecordStore = rawRecordStore,
        loadWakeKeywords = loadWakeKeywords,
        recordTranslator = recordTranslator,
        executeAfterInit = executeAfterInit,
        nativeValidateStructure = nativeValidateStructure,
        nativeValidateLogic = nativeValidateLogic,
        nativeIngestSingleTxtReplaceMonth = nativeIngestSingleTxtReplaceMonth
    )

    suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val paths = ensureRuntimePaths()
            val result = rawRecordStore.ensureCurrentMonthFile(paths.inputRootPath)
            RecordActionResult(
                ok = true,
                message = "Created month TXT -> ${result.monthFile.name}" +
                    if (result.created) " (new file)" else " (already exists)"
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Create current month TXT failed", error = error)
        }
    }

    suspend fun createMonthTxt(month: String): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val parsed = month.trim().split("-")
            if (parsed.size != 2) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month format: $month. Expected YYYY-MM."
                )
            }
            val year = parsed[0].toIntOrNull()
            val monthValue = parsed[1].toIntOrNull()
            if (year == null || monthValue == null || monthValue !in 1..12) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month format: $month. Expected YYYY-MM with month 01-12."
                )
            }
            val paths = ensureRuntimePaths()
            val result = rawRecordStore.ensureMonthFile(paths.inputRootPath, year, monthValue)
            RecordActionResult(
                ok = true,
                message = "Created month TXT -> ${result.monthFile.name}" +
                    if (result.created) " (new file)" else " (already exists)"
            )
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Create month TXT failed for $month", error = error)
        }
    }

    suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            Log.i(
                RECORD_LOG_TAG,
                "record.start activity=${activityName.trim()} targetDate=$targetDateIso " +
                    "preferredTxtPath=$preferredTxtPath timeOrderMode=$timeOrderMode"
            )
            val registration = ensureActivityHierarchyEntry(activityName)
            Log.i(
                RECORD_LOG_TAG,
                "record.hierarchy_registration ok=${registration.ok} message=${registration.message}"
            )
            if (!registration.ok) {
                return@withContext buildRecordActionFailure(
                    prefix = "Register activity hierarchy entry failed",
                    error = IllegalStateException(registration.message)
                )
            }
            atomicFlow.recordNow(
                activityName = activityName,
                remark = remark,
                targetDateIso = targetDateIso,
                preferredTxtPath = preferredTxtPath,
                timeOrderMode = timeOrderMode
            ).withActivityHierarchyRegistration(registration)
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Record failed", error = error)
        }
    }

    suspend fun updateActivityRemark(
        targetDateIso: String,
        logicalId: Long,
        remark: String,
        preferredTxtPath: String?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            Log.i(
                REMARK_UPDATE_LOG_TAG,
                "submit targetDate=$targetDateIso logicalId=$logicalId " +
                    "remarkLength=${remark.length} preferredTxtPath=$preferredTxtPath"
            )
            val response = executeAfterInit("native_update_activity_remark_atomically") {
                nativeUpdateActivityRemarkAtomically(
                    targetDateIso,
                    logicalId,
                    remark,
                    preferredTxtPath,
                    NativeBridge.DATE_CHECK_NONE
                )
            }
            val payload = responseCodec.parse(response.rawResponse)
            val detail = atomicRecordCodec.parse(payload.content)
            Log.i(
                REMARK_UPDATE_LOG_TAG,
                "response logicalId=$logicalId initialized=${response.initialized} " +
                    "operationOk=${response.operationOk} payloadOk=${payload.ok} " +
                    "detailOk=${detail?.ok} error=${payload.errorMessage}"
            )
            if (response.initialized && response.operationOk && payload.ok && detail?.ok == true) {
                RecordActionResult(ok = true, message = detail.message, operationId = detail.operationId)
            } else {
                RecordActionResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank { detail?.message ?: "Activity remark update failed." },
                    operationId = detail?.operationId.orEmpty()
                )
            }
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Update activity remark failed", error = error)
        }
    }

    suspend fun updateDayRemark(
        targetDateIso: String,
        remark: String,
        preferredTxtPath: String?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val response = executeAfterInit("native_update_day_remark_atomically") {
                nativeUpdateDayRemarkAtomically(
                    targetDateIso,
                    remark,
                    preferredTxtPath,
                    NativeBridge.DATE_CHECK_NONE
                )
            }
            val payload = responseCodec.parse(response.rawResponse)
            val detail = atomicRecordCodec.parse(payload.content)
            if (response.initialized && response.operationOk && payload.ok && detail?.ok == true) {
                RecordActionResult(ok = true, message = detail.message, operationId = detail.operationId)
            } else {
                RecordActionResult(
                    ok = false,
                    message = payload.errorMessage.ifBlank { detail?.message ?: "Day remark update failed." },
                    operationId = detail?.operationId.orEmpty()
                )
            }
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Update day remark failed", error = error)
        }
    }

    suspend fun recordInterval(
        activityName: String,
        startTime: String,
        endTime: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val logicalDateResult = parseLogicalDate(targetDateIso)
            if (!logicalDateResult.ok) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = logicalDateResult.message
                )
            }

            val normalizedActivity = rawRecordStore.normalizeActivityName(activityName)
            if (normalizedActivity.isEmpty()) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: activity token is required."
                )
            }

            val registration = ensureActivityHierarchyEntry(normalizedActivity)
            if (!registration.ok) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: ${registration.message}"
                )
            }

            val normalizedRemark = rawRecordStore.normalizeRemark(remark)
            val normalizedStart = normalizeToHhmmss(startTime)
            val normalizedEnd = normalizeToHhmmss(endTime)
            if (normalizedStart == null || normalizedEnd == null) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: start/end must use HHMM or HHMMSS."
                )
            }

            val paths = ensureRuntimePaths()
            val logicalDate = logicalDateResult.date!!
            val target = resolveRecordTarget(paths, logicalDate, preferredTxtPath)
            val targetRelativePath = target.preferredInnerPath
                ?: buildMonthRelativePath(logicalDate.substring(0, 7))
            val logicalYear = logicalDate.substring(0, 4).toInt()
            val logicalMonth = logicalDate.substring(5, 7).toInt()
            rawRecordStore.ensureMonthFile(
                inputRootPath = paths.inputRootPath,
                year = logicalYear,
                month = logicalMonth
            )

            val storage = ensureTextStorage()
            val readResult = storage.readTxtFile(targetRelativePath)
            if (!readResult.ok) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: failed to read TXT -> $targetRelativePath (${readResult.message})"
                )
            }

            val selectedMonth = logicalDate.substring(0, 7)
            val markerResult = defaultTxtDayMarker(selectedMonth, logicalDate)
            if (!markerResult.ok) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: ${markerResult.message}"
                )
            }

            val dayBlockResult = resolveTxtDayBlock(
                readResult.content,
                markerResult.normalizedDayMarker,
                selectedMonth
            )
            if (!dayBlockResult.ok) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Record blocked: ${dayBlockResult.message}"
                )
            }

            val eventLine = rawRecordStore.buildRawIntervalEventLine(
                startHhmm = normalizedStart,
                endHhmm = normalizedEnd,
                activity = normalizedActivity,
                remark = normalizedRemark
            )
            val updatedContent = if (dayBlockResult.found) {
                val replaced = replaceTxtDayBlock(
                    readResult.content,
                    markerResult.normalizedDayMarker,
                    appendEventLine(dayBlockResult.dayBody, eventLine)
                )
                if (!replaced.ok) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "Record blocked: ${replaced.message}"
                    )
                }
                replaced.updatedContent
            } else {
                appendMissingDayBlock(
                    content = readResult.content,
                    dayMarker = markerResult.normalizedDayMarker,
                    eventLine = eventLine
                )
            }

            txtSaveAndSyncFlow.saveTxtFileAndSync(
                relativePath = targetRelativePath,
                content = updatedContent
            ).withActivityHierarchyRegistration(registration)
        } catch (error: Exception) {
            buildRecordActionFailure(prefix = "Record interval failed", error = error)
        }
    }

    suspend fun syncLiveToDatabase(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            syncFlow.syncLiveToDatabase()
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "syncLiveToDatabase failed", error = error)
        }
    }

    suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        txtSaveAndSyncFlow.saveTxtFileAndSync(
            relativePath = relativePath,
            content = content
        )

    private fun appendEventLine(dayBody: String, eventLine: String): String {
        val trimmed = dayBody.trimEnd('\n', '\r')
        return if (trimmed.isEmpty()) {
            "$eventLine\n"
        } else {
            "$trimmed\n$eventLine\n"
        }
    }

    private fun appendMissingDayBlock(content: String, dayMarker: String, eventLine: String): String {
        val canonicalContent = CanonicalTextCodec.canonicalizeText(content).trimEnd('\n', '\r')
        val dayMarkerLine = buildDayMarkerLine(dayMarker)
        return if (canonicalContent.isEmpty()) {
            "$dayMarkerLine\n$eventLine\n"
        } else {
            "$canonicalContent\n\n$dayMarkerLine\n$eventLine\n"
        }
    }

    // dayMarker stays MMDD at the UI/API boundary; dMMDD is only the raw TXT
    // header written to files so it cannot collide with HHMM event records.
    private fun buildDayMarkerLine(dayMarker: String): String = "d$dayMarker"

    private fun RecordActionResult.withActivityHierarchyRegistration(
        registration: ActivityHierarchyAutoRegistrationResult
    ): RecordActionResult = copy(
        activityHierarchyCreated = registration.created,
        activityHierarchyCategory = registration.categoryName,
        activityHierarchyActivity = registration.activityName
    )

    private fun normalizeToHhmmss(rawTime: String): String? {
        val time = rawTime.trim()
        if ((time.length != 4 && time.length != 6) || !time.all { it.isDigit() }) {
            return null
        }
        val hours = time.substring(0, 2).toIntOrNull() ?: return null
        val minutes = time.substring(2, 4).toIntOrNull() ?: return null
        val seconds = if (time.length == 6) time.substring(4, 6).toIntOrNull() else 0
        if (hours !in 0..23 || minutes !in 0..59 || seconds !in 0..59) {
            return null
        }
        return if (time.length == 4) "$time" + "00" else time
    }
}
