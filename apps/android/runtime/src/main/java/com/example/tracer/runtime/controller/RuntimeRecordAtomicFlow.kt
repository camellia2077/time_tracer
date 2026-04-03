package com.example.tracer

internal class RuntimeRecordAtomicFlow(
    private val responseCodec: NativeResponseCodec,
    private val atomicRecordCodec: NativeAtomicRecordCodec,
    private val executeAfterInit: (
        operationName: String,
        action: (RuntimePaths) -> String
    ) -> NativeCallResult,
    private val nativeRecordActivityAtomically: (
        targetDateIso: String,
        rawActivityName: String,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int,
        timeOrderMode: RecordTimeOrderMode
    ) -> String
) {
    fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult {
        val logicalDateResult = parseLogicalDate(targetDateIso)
        if (!logicalDateResult.ok || logicalDateResult.date == null) {
            return RecordActionResult(
                ok = false,
                message = logicalDateResult.message
            )
        }
        val logicalDate = logicalDateResult.date

        val atomicResult = executeAfterInit("native_record_activity_atomically") {
            nativeRecordActivityAtomically(
                logicalDate,
                activityName,
                remark,
                preferredTxtPath,
                NativeBridge.DATE_CHECK_NONE,
                timeOrderMode
            )
        }
        val payload = responseCodec.parse(atomicResult.rawResponse)
        val atomicPayload = atomicRecordCodec.parse(payload.content)
        val baseMessage = atomicPayload?.message
            ?.takeIf { it.isNotBlank() }
            ?: payload.errorMessage.takeIf { it.isNotBlank() }
            ?: if (atomicResult.operationOk) "record: ok\nsync: ok" else "Record failed."

        if (!atomicResult.initialized || !atomicResult.operationOk) {
            return RecordActionResult(
                ok = false,
                message = appendFailureContext(
                    message = baseMessage,
                    operationId = atomicResult.operationId,
                    errorLogPath = atomicResult.errorLogPath
                ),
                operationId = atomicResult.operationId
            )
        }

        return RecordActionResult(
            ok = true,
            message = baseMessage,
            operationId = atomicResult.operationId
        )
    }
}

