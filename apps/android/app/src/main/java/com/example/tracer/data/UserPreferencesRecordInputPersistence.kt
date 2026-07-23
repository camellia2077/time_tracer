package com.example.tracer.data

import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.RecordAuthoringMode
import com.example.tracer.RecordInputPersistence
import com.example.tracer.TxtOutputMode

class UserPreferencesRecordInputPersistence(
    private val repository: UserPreferencesRepository
) : RecordInputPersistence {
    override suspend fun persistLastAuthoringMode(mode: RecordAuthoringMode) {
        repository.setRecordLastAuthoringMode(mode)
    }

    override suspend fun persistLastTxtOutputMode(mode: TxtOutputMode) {
        repository.setRecordLastTxtOutputMode(mode)
    }

    override suspend fun persistDraft(draft: PersistedRecordInputDraft) {
        repository.saveRecordDraft(draft)
    }

    override suspend fun clearDraft() {
        repository.clearRecordDraft()
    }
}
