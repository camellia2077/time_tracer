package com.example.tracer.data

import androidx.datastore.preferences.core.PreferenceDataStoreFactory
import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.RecordAuthoringMode
import com.example.tracer.RecordLogicalDayTarget
import com.example.tracer.RecordFrequentOutputMode
import com.example.tracer.CanonicalCatalogSource
import com.example.tracer.TxtOutputMode
import com.example.tracer.InsightsChartSemanticMode
import com.example.tracer.InsightsParameterSection
import com.example.tracer.InsightsPiePalettePreset
import com.example.tracer.InsightsResultDisplayMode
import com.example.tracer.InsightsMode
import com.example.tracer.InsightsActivityView
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import java.io.File
import kotlin.io.path.createTempDirectory

class UserPreferencesRepositoryTest {
    @Test
    fun appLanguage_defaultsToSystem_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_app_language",
            scope = backgroundScope
        )

        assertEquals(AppLanguage.System, repository.appLanguage.first())

        repository.setAppLanguage(AppLanguage.Japanese)

        assertEquals(AppLanguage.Japanese, repository.appLanguage.first())
    }

    @Test
    fun promptBeforeUnconfiguredActivityRecord_defaultsOff_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_unconfigured_activity_warning",
            scope = backgroundScope
        )

        assertEquals(
            UserPreferencesRepository.DEFAULT_PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD,
            repository.promptBeforeUnconfiguredActivityRecord.first()
        )

        repository.setPromptBeforeUnconfiguredActivityRecord(true)

        assertEquals(true, repository.promptBeforeUnconfiguredActivityRecord.first())
    }

    @Test
    fun recordFrequentPreferences_usesEmptyQuickActivities_whenQuickActivitiesNotConfigured() = runTest {
        val repository = buildRepository(
            testName = "missing_quick_activities",
            scope = backgroundScope
        )

        val preferences = repository.recordFrequentPreferences.first()

        assertEquals(
            emptyList<String>(),
            preferences.quickActivities
        )
        assertEquals(
            UserPreferencesRepository.DEFAULT_RECORD_FREQUENT_OUTPUT_MODE,
            preferences.outputMode
        )
        assertEquals(
            UserPreferencesRepository.DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE,
            preferences.canonicalCatalogDisplayMode
        )
        assertEquals(
            CanonicalCatalogSource.TREE,
            preferences.canonicalCatalogSource
        )
    }

    @Test
    fun setRecordQuickActivities_emptyList_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository(
            testName = "empty_quick_activities",
            scope = backgroundScope
        )

        repository.setRecordQuickActivities(emptyList())
        val preferences = repository.recordFrequentPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun setRecordQuickActivities_blankValues_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository(
            testName = "blank_quick_activities",
            scope = backgroundScope
        )

        repository.setRecordQuickActivities(listOf(" ", ""))
        val preferences = repository.recordFrequentPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun insightsPiePalettePreset_defaultsToVivid() = runTest {
        val repository = buildRepository(
            testName = "default_pie_palette",
            scope = backgroundScope
        )

        assertEquals(
            InsightsPiePalettePreset.VIVID,
            repository.insightsPiePalettePreset.first()
        )
    }

    @Test
    fun setInsightsPiePalettePreset_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_pie_palette",
            scope = backgroundScope
        )

        repository.setInsightsPiePalettePreset(InsightsPiePalettePreset.EDITORIAL)

        assertEquals(
            InsightsPiePalettePreset.EDITORIAL,
            repository.insightsPiePalettePreset.first()
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
    fun setRecordLastTxtOutputMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_txt_output_mode",
            scope = backgroundScope
        )

        repository.setRecordLastTxtOutputMode(TxtOutputMode.DAY)

        assertEquals(
            TxtOutputMode.DAY,
            repository.recordPersistedInput.first().lastTxtOutputMode
        )
    }

    @Test
    fun recordPersistedInput_defaultsToInterval_whenModeHasNotBeenSelected() = runTest {
        val repository = buildRepository(
            testName = "default_record_authoring_mode",
            scope = backgroundScope
        )

        assertEquals(
            RecordAuthoringMode.INTERVAL,
            repository.recordPersistedInput.first().lastAuthoringMode
        )
        assertEquals(
            TxtOutputMode.DAY,
            repository.recordPersistedInput.first().lastTxtOutputMode
        )
    }

    @Test
    fun setRecordFrequentOutputMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_frequent_output_mode",
            scope = backgroundScope
        )

        repository.setRecordFrequentOutputMode(RecordFrequentOutputMode.ALIAS)

        assertEquals(
            RecordFrequentOutputMode.ALIAS,
            repository.recordFrequentPreferences.first().outputMode
        )
    }

    @Test
    fun insightsChartSemanticMode_defaultsToBreakdown_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_chart_semantic_mode",
            scope = backgroundScope
        )

        assertEquals(
            InsightsChartSemanticMode.COMPOSITION,
            repository.insightsChartSemanticMode.first()
        )

        repository.setInsightsChartSemanticMode(InsightsChartSemanticMode.TREND)

        assertEquals(
            InsightsChartSemanticMode.TREND,
            repository.insightsChartSemanticMode.first()
        )
    }

    @Test
    fun insightsHeatmapPaletteName_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_heatmap_palette",
            scope = backgroundScope
        )

        repository.setInsightsHeatmapPaletteName("BLUE_LIGHT")

        assertEquals("BLUE_LIGHT", repository.insightsHeatmapPaletteName.first())
    }

    @Test
    fun statusConfigs_areStoredIndependentlyForEachInsightsMode() = runTest {
        val repository = buildRepository(
            testName = "persist_daily_statuses",
            scope = backgroundScope
        )
        val expected = DailyStatusConfig(
            statuses = listOf(
                DailyStatusDefinition("study__math", "Study\tMath", "study/math")
            )
        )

        repository.setStatusConfig(InsightsMode.DAY, expected)
        repository.setStatusConfig(
            InsightsMode.WEEK,
            DailyStatusConfig(statuses = listOf(DailyStatusDefinition("exercise", "Exercise", "exercise")))
        )

        val configs = repository.insightsStatusConfigs.first()
        assertEquals(expected, configs[InsightsMode.DAY])
        assertEquals(listOf("exercise"), configs[InsightsMode.WEEK].statuses.map { it.id })
        assertEquals(emptyList<DailyStatusDefinition>(), configs[InsightsMode.MONTH].statuses)
    }

    @Test
    fun insightsChartTrendRoot_defaultsToAllActivities_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_chart_trend_root",
            scope = backgroundScope
        )

        assertEquals("", repository.insightsChartTrendRoot.first())

        repository.setInsightsChartTrendRoot(" study ")

        assertEquals("study", repository.insightsChartTrendRoot.first())
    }

    @Test
    fun insightsResultDisplayMode_isStoredIndependentlyForEachInsightsMode() = runTest {
        val repository = buildRepository(
            testName = "persist_insights_result_display_mode",
            scope = backgroundScope
        )

        assertEquals(InsightsResultDisplayMode.TEXT, repository.insightsResultDisplayMode.first())

        repository.setInsightsResultDisplayMode(InsightsResultDisplayMode.CHART)

        assertEquals(InsightsResultDisplayMode.CHART, repository.insightsResultDisplayMode.first())

        repository.setInsightsMode(InsightsMode.MONTH)
        assertEquals(InsightsResultDisplayMode.TEXT, repository.insightsResultDisplayMode.first())

        repository.setInsightsResultDisplayMode(InsightsResultDisplayMode.CHART)
        repository.setInsightsMode(InsightsMode.DAY)

        assertEquals(InsightsResultDisplayMode.CHART, repository.insightsResultDisplayMode.first())
    }

    @Test
    fun insightsParameterSection_defaultsToDay_andPersistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_insights_parameter_section",
            scope = backgroundScope
        )

        assertEquals(InsightsParameterSection.DAY, repository.insightsParameterSection.first())

        repository.setInsightsParameterSection(InsightsParameterSection.ACTIVITIES)

        assertEquals(InsightsParameterSection.ACTIVITIES, repository.insightsParameterSection.first())
    }

    @Test
    fun setRecordCanonicalCatalogSource_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_canonical_catalog_source",
            scope = backgroundScope
        )

        repository.setRecordCanonicalCatalogSource(CanonicalCatalogSource.CATEGORIES)

        assertEquals(
            CanonicalCatalogSource.CATEGORIES,
            repository.recordFrequentPreferences.first().canonicalCatalogSource
        )
    }

    @Test
    fun insightsActivitiesView_defaultsAndPersistsDayAndPeriodSelectionsIndependently() = runTest {
        val repository = buildRepository(
            testName = "persist_insights_activities_view",
            scope = backgroundScope
        )

        assertEquals(InsightsActivityView.RECORDS, repository.insightsDayActivitiesView.first())
        assertEquals(InsightsActivityView.OVERVIEW, repository.insightsPeriodActivitiesView.first())

        repository.setInsightsDayActivitiesView(InsightsActivityView.OVERVIEW)
        repository.setInsightsPeriodActivitiesView(InsightsActivityView.RECORDS)

        assertEquals(InsightsActivityView.OVERVIEW, repository.insightsDayActivitiesView.first())
        assertEquals(InsightsActivityView.RECORDS, repository.insightsPeriodActivitiesView.first())
    }

    @Test
    fun insightsTimeParametersExpanded_defaultsExpanded_andPersistsCollapsedState() = runTest {
        val repository = buildRepository(
            testName = "persist_insights_time_parameters_expanded",
            scope = backgroundScope
        )

        assertEquals(true, repository.insightsTimeParametersExpanded.first())

        repository.setInsightsTimeParametersExpanded(false)

        assertEquals(false, repository.insightsTimeParametersExpanded.first())
    }

    @Test
    fun recordQuickAccessCardExpanded_defaultsExpanded_andPersistsCollapsedState() = runTest {
        val repository = buildRepository(
            testName = "persist_record_quick_access_card_expanded",
            scope = backgroundScope
        )

        assertEquals(
            true,
            repository.recordFrequentPreferences.first().quickAccessCardExpanded
        )

        repository.setRecordQuickAccessCardExpanded(false)

        assertEquals(
            false,
            repository.recordFrequentPreferences.first().quickAccessCardExpanded
        )
    }

    @Test
    fun configCardExpansionPreferences_defaultExpanded_andPersistEachCardIndependently() = runTest {
        val repository = buildRepository(
            testName = "persist_config_card_expansion",
            scope = backgroundScope
        )

        assertEquals(
            true,
            repository.configCardExpansionPreferences.first().isExpanded(ConfigCard.APPEARANCE)
        )

        repository.setConfigCardExpanded(ConfigCard.APPEARANCE, false)
        repository.setConfigCardExpanded(ConfigCard.DATA_MANAGEMENT, false)

        val preferences = repository.configCardExpansionPreferences.first()
        assertEquals(false, preferences.appearanceExpanded)
        assertEquals(false, preferences.dataManagementExpanded)
        assertEquals(true, preferences.applicationPreferencesExpanded)
        assertEquals(true, preferences.insightsSettingsExpanded)
        assertEquals(true, preferences.aboutExpanded)
    }

    @Test
    fun setRecordCanonicalCatalogDisplayMode_persistsSelection() = runTest {
        val repository = buildRepository(
            testName = "persist_record_canonical_catalog_display_mode",
            scope = backgroundScope
        )

        repository.setRecordCanonicalCatalogDisplayMode(RecordFrequentOutputMode.ALIAS)

        assertEquals(
            RecordFrequentOutputMode.ALIAS,
            repository.recordFrequentPreferences.first().canonicalCatalogDisplayMode
        )
    }

    @Test
    fun recordFrequentPreferences_defaultsCollapsedCanonicalRootPathsToEmpty() = runTest {
        val repository = buildRepository(
            testName = "default_collapsed_canonical_roots",
            scope = backgroundScope
        )

        assertEquals(
            emptySet<String>(),
            repository.recordFrequentPreferences.first().collapsedCanonicalRootPaths
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
            repository.recordFrequentPreferences.first().collapsedCanonicalRootPaths
        )
    }

    @Test
    fun recordFrequentPreferences_defaultsOrderedCanonicalRootPathsToEmpty() = runTest {
        val repository = buildRepository(
            testName = "default_ordered_canonical_roots",
            scope = backgroundScope
        )

        assertEquals(
            emptyList<String>(),
            repository.recordFrequentPreferences.first().orderedCanonicalRootPaths
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
            repository.recordFrequentPreferences.first().orderedCanonicalRootPaths
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
                intervalStart = "09:00:00",
                intervalEnd = "10:30:00",
                attributionDateIso = "2026-03-28",
                logicalDayTarget = RecordLogicalDayTarget.YESTERDAY
            )
        )

        val persisted = repository.recordPersistedInput.first()
        assertEquals("study", persisted.draft?.recordContent)
        assertEquals("focused block", persisted.draft?.recordRemark)
        assertEquals("09:00:00", persisted.draft?.intervalStart)
        assertEquals("10:30:00", persisted.draft?.intervalEnd)
        assertEquals("2026-03-28", persisted.draft?.attributionDateIso)
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
