package com.example.tracer.data

import androidx.datastore.preferences.core.PreferenceDataStoreFactory
import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.RecordAuthoringMode
import com.example.tracer.RecordLogicalDayTarget
import com.example.tracer.ReportPiePalettePreset
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import java.io.File
import kotlin.io.path.createTempDirectory

class UserPreferencesRepositoryTest {
    @Test
    fun recordSuggestionPreferences_usesDefaultQuickActivities_whenQuickActivitiesNotConfigured() = runTest {
        val repository = buildRepository(
            testName = "missing_quick_activities",
            scope = backgroundScope
        )

        val preferences = repository.recordSuggestionPreferences.first()

        assertEquals(
            UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACTIVITIES,
            preferences.quickActivities
        )
    }

    @Test
    fun setRecordQuickActivities_emptyList_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository(
            testName = "empty_quick_activities",
            scope = backgroundScope
        )

        repository.setRecordQuickActivities(emptyList())
        val preferences = repository.recordSuggestionPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun setRecordQuickActivities_blankValues_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository(
            testName = "blank_quick_activities",
            scope = backgroundScope
        )

        repository.setRecordQuickActivities(listOf(" ", ""))
        val preferences = repository.recordSuggestionPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun reportPiePalettePreset_defaultsToSoft() = runTest {
        val repository = buildRepository(
            testName = "default_pie_palette",
            scope = backgroundScope
        )

        assertEquals(
            UserPreferencesRepository.DEFAULT_REPORT_PIE_PALETTE_PRESET,
            repository.reportPiePalettePreset.first()
        )
    }

    @Test
    fun setReportPiePalettePreset_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_pie_palette",
            scope = backgroundScope
        )

        repository.setReportPiePalettePreset(ReportPiePalettePreset.EDITORIAL)

        assertEquals(
            ReportPiePalettePreset.EDITORIAL,
            repository.reportPiePalettePreset.first()
        )
    }

    @Test
    fun setRecordLastAuthoringMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_authoring_mode",
            scope = backgroundScope
        )

        repository.setRecordLastAuthoringMode(RecordAuthoringMode.INTERVAL)

        assertEquals(
            RecordAuthoringMode.INTERVAL,
            repository.recordPersistedInput.first().lastAuthoringMode
        )
    }

    @Test
    fun saveRecordDraft_persistsDraftFieldsAndLogicalDay() = runTest {
        val repository = buildRepository(
            testName = "persist_record_draft",
            scope = backgroundScope
        )

        repository.saveRecordDraft(
            PersistedRecordInputDraft(
                recordContent = "study",
                recordRemark = "focused block",
                intervalStart = "0900",
                intervalEnd = "1030",
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY
            )
        )

        val persisted = repository.recordPersistedInput.first()
        assertEquals("study", persisted.draft?.recordContent)
        assertEquals("focused block", persisted.draft?.recordRemark)
        assertEquals("0900", persisted.draft?.intervalStart)
        assertEquals("1030", persisted.draft?.intervalEnd)
        assertEquals(RecordLogicalDayTarget.YESTERDAY, persisted.draft?.logicalDayTarget)
    }

    @Test
    fun clearRecordDraft_removesPersistedDraftButKeepsMode() = runTest {
        val repository = buildRepository(
            testName = "clear_record_draft",
            scope = backgroundScope
        )

        repository.setRecordLastAuthoringMode(RecordAuthoringMode.INTERVAL)
        repository.saveRecordDraft(
            PersistedRecordInputDraft(
                recordContent = "study",
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY
            )
        )

        repository.clearRecordDraft()

        val persisted = repository.recordPersistedInput.first()
        assertEquals(RecordAuthoringMode.INTERVAL, persisted.lastAuthoringMode)
        assertEquals(null, persisted.draft)
    }

    private fun buildRepository(testName: String, scope: CoroutineScope): UserPreferencesRepository {
        val tempDir = createTempDirectory(prefix = "user_prefs_$testName").toFile().apply {
            deleteOnExit()
        }
        // Use a not-yet-created file inside a unique temp directory. This avoids
        // Windows-specific file locking issues from handing DataStore a pre-created
        // temp file that may still be held by the test process/runtime.
        val prefsFile = File(tempDir, "settings.preferences_pb")
        val dataStore = PreferenceDataStoreFactory.create(
            scope = scope,
            produceFile = { prefsFile }
        )
        return UserPreferencesRepository(dataStore)
    }
}
