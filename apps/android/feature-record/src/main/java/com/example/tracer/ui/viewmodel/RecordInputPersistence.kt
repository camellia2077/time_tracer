package com.example.tracer

data class PersistedRecordInputDraft(
    val recordContent: String = "",
    val recordRemark: String = "",
    val intervalStart: String = "",
    val intervalEnd: String = "",
    val intervalStartedAtEpochMs: Long = 0L,
    val attributionDateIso: String = "",
    val logicalDayTarget: RecordLogicalDayTarget = RecordLogicalDayTarget.TODAY
)

data class PersistedRecordInputSnapshot(
    val lastAuthoringMode: RecordAuthoringMode = RecordAuthoringMode.INTERVAL,
    val lastTxtOutputMode: TxtOutputMode = TxtOutputMode.DAY,
    val draft: PersistedRecordInputDraft? = null
)

interface RecordInputPersistence {
    suspend fun persistLastAuthoringMode(mode: RecordAuthoringMode)

    suspend fun persistLastTxtOutputMode(mode: TxtOutputMode)

    suspend fun persistDraft(draft: PersistedRecordInputDraft)

    suspend fun clearDraft()
}

internal object NoOpRecordInputPersistence : RecordInputPersistence {
    override suspend fun persistLastAuthoringMode(mode: RecordAuthoringMode) = Unit

    override suspend fun persistLastTxtOutputMode(mode: TxtOutputMode) = Unit

    override suspend fun persistDraft(draft: PersistedRecordInputDraft) = Unit

    override suspend fun clearDraft() = Unit
}
