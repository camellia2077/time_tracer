package com.example.tracer.data

import androidx.datastore.preferences.core.PreferenceDataStoreFactory
import com.example.tracer.ReportPiePalettePreset
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import java.io.File

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

    private fun buildRepository(testName: String, scope: CoroutineScope): UserPreferencesRepository {
        val prefsFile = File.createTempFile("user_prefs_$testName", ".preferences_pb").apply {
            deleteOnExit()
        }
        val dataStore = PreferenceDataStoreFactory.create(
            scope = scope,
            produceFile = { prefsFile }
        )
        return UserPreferencesRepository(dataStore)
    }
}
