package com.example.tracer.data

import androidx.datastore.preferences.core.PreferenceDataStoreFactory
import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.RecordAuthoringMode
import com.example.tracer.RecordLogicalDayTarget
import com.example.tracer.RecordSuggestionOutputMode
import com.example.tracer.ReportChartSemanticMode
import com.example.tracer.ReportParameterSection
import com.example.tracer.ReportPiePalettePreset
import com.example.tracer.ReportResultDisplayMode
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
        assertEquals(
            UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_OUTPUT_MODE,
            preferences.outputMode
        )
        assertEquals(
            UserPreferencesRepository.DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE,
            preferences.canonicalCatalogDisplayMode
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
    fun setRecordSuggestOutputMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_suggest_output_mode",
            scope = backgroundScope
        )

        repository.setRecordSuggestOutputMode(RecordSuggestionOutputMode.ALIAS)

        assertEquals(
            RecordSuggestionOutputMode.ALIAS,
            repository.recordSuggestionPreferences.first().outputMode
        )
    }

    @Test
    fun reportChartSemanticMode_defaultsToBreakdown_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_chart_semantic_mode",
            scope = backgroundScope
        )

        assertEquals(
            ReportChartSemanticMode.COMPOSITION,
            repository.reportChartSemanticMode.first()
        )

        repository.setReportChartSemanticMode(ReportChartSemanticMode.TREND)

        assertEquals(
            ReportChartSemanticMode.TREND,
            repository.reportChartSemanticMode.first()
        )
    }

    @Test
    fun reportResultDisplayMode_defaultsToText_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_report_result_display_mode",
            scope = backgroundScope
        )

        assertEquals(ReportResultDisplayMode.TEXT, repository.reportResultDisplayMode.first())

        repository.setReportResultDisplayMode(ReportResultDisplayMode.CHART)

        assertEquals(ReportResultDisplayMode.CHART, repository.reportResultDisplayMode.first())
    }

    @Test
    fun reportParameterSection_defaultsToDay_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_report_parameter_section",
            scope = backgroundScope
        )

        assertEquals(ReportParameterSection.DAY, repository.reportParameterSection.first())

        repository.setReportParameterSection(ReportParameterSection.STATS)

        assertEquals(ReportParameterSection.STATS, repository.reportParameterSection.first())
    }

    @Test
    fun setRecordCanonicalCatalogDisplayMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_canonical_catalog_display_mode",
            scope = backgroundScope
        )

        repository.setRecordCanonicalCatalogDisplayMode(RecordSuggestionOutputMode.ALIAS)

        assertEquals(
            RecordSuggestionOutputMode.ALIAS,
            repository.recordSuggestionPreferences.first().canonicalCatalogDisplayMode
        )
    }

    @Test
    fun recordSuggestionPreferences_defaultsCollapsedCanonicalRootPathsToEmpty() = runTest {
        val repository = buildRepository(
            testName = "default_collapsed_canonical_roots",
            scope = backgroundScope
        )

        assertEquals(
            emptySet<String>(),
            repository.recordSuggestionPreferences.first().collapsedCanonicalRootPaths
        )
    }

    @Test
    fun setRecordCollapsedCanonicalRootPaths_persistsNormalizedRoots() = runTest {
        val repository = buildRepository(
            testName = "persist_collapsed_canonical_roots",
            scope = backgroundScope
        )

        repository.setRecordCollapsedCanonicalRootPaths(
            setOf("study", " study/math ", "", "study")
        )

        assertEquals(
            linkedSetOf("study", "study/math"),
            repository.recordSuggestionPreferences.first().collapsedCanonicalRootPaths
        )
    }

    @Test
    fun recordSuggestionPreferences_defaultsOrderedCanonicalRootPathsToEmpty() = runTest {
        val repository = buildRepository(
            testName = "default_ordered_canonical_roots",
            scope = backgroundScope
        )

        assertEquals(
            emptyList<String>(),
            repository.recordSuggestionPreferences.first().orderedCanonicalRootPaths
        )
    }

    @Test
    fun setRecordOrderedCanonicalRootPaths_persistsNormalizedOrder() = runTest {
        val repository = buildRepository(
            testName = "persist_ordered_canonical_roots",
            scope = backgroundScope
        )

        repository.setRecordOrderedCanonicalRootPaths(
            listOf("study", " study/math ", "", "study")
        )

        assertEquals(
            listOf("study", "study/math"),
            repository.recordSuggestionPreferences.first().orderedCanonicalRootPaths
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
