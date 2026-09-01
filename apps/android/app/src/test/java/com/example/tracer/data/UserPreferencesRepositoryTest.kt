package com.example.tracer.data

import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.emptyPreferences
import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.RecordAuthoringMode
import com.example.tracer.RecordLogicalDayTarget
import com.example.tracer.RecordFrequentOutputMode
import com.example.tracer.CanonicalCatalogSource
import com.example.tracer.TxtOutputMode
import com.example.tracer.InsightsChartSemanticMode
import com.example.tracer.InsightsComparisonColorScheme
import com.example.tracer.InsightsComparisonIndicatorStyle
import com.example.tracer.InsightsParameterSection
import com.example.tracer.InsightsPiePalettePreset
import com.example.tracer.InsightsResultDisplayMode
import com.example.tracer.InsightsMode
import com.example.tracer.InsightsActivityView
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock

class UserPreferencesRepositoryTest {
    @Test
    fun appLanguage_defaultsToSystem_andPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(AppLanguage.System, repository.appLanguage.first())

        repository.setAppLanguage(AppLanguage.Japanese)

        assertEquals(AppLanguage.Japanese, repository.appLanguage.first())
    }

    @Test
    fun promptBeforeUnconfiguredActivityRecord_defaultsOff_andPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(
            UserPreferencesRepository.DEFAULT_PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD,
            repository.promptBeforeUnconfiguredActivityRecord.first()
        )

        repository.setPromptBeforeUnconfiguredActivityRecord(true)

        assertEquals(true, repository.promptBeforeUnconfiguredActivityRecord.first())
    }

    @Test
    fun recordFrequentPreferences_usesEmptyQuickActivities_whenQuickActivitiesNotConfigured() = runTest {
        val repository = buildRepository()

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
    fun pageTransitionPreferences_defaultToQuickFade_andPersistSelections() = runTest {
        val repository = buildRepository()

        assertEquals(
            UserPreferencesRepository.DEFAULT_PAGE_TRANSITION_STYLE,
            repository.pageTransitionStyle.first()
        )

        repository.setPageTransitionStyle(PageTransitionStyle.NONE)

        assertEquals(PageTransitionStyle.NONE, repository.pageTransitionStyle.first())
    }

    @Test
    fun timeDisplayMode_defaultsToTwentyFourHour_andPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(
            UserPreferencesRepository.DEFAULT_TIME_DISPLAY_MODE,
            repository.timeDisplayMode.first()
        )

        repository.setTimeDisplayMode(TimeDisplayMode.TWELVE_HOUR)

        assertEquals(TimeDisplayMode.TWELVE_HOUR, repository.timeDisplayMode.first())
    }

    @Test
    fun recordFrequentPreferences_persistsZeroLookbackDaysAndTopN() = runTest {
        val repository = buildRepository()

        repository.setRecordFrequentLookbackDays(0)
        repository.setRecordFrequentTopN(0)

        val preferences = repository.recordFrequentPreferences.first()
        assertEquals(0, preferences.lookbackDays)
        assertEquals(0, preferences.topN)
    }

    @Test
    fun setRecordQuickActivities_emptyList_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository()

        repository.setRecordQuickActivities(emptyList())
        val preferences = repository.recordFrequentPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun setRecordQuickActivities_blankValues_keepsQuickActivitiesEmpty() = runTest {
        val repository = buildRepository()

        repository.setRecordQuickActivities(listOf(" ", ""))
        val preferences = repository.recordFrequentPreferences.first()

        assertEquals(emptyList<String>(), preferences.quickActivities)
    }

    @Test
    fun insightsPiePalettePreset_defaultsToVivid() = runTest {
        val repository = buildRepository()

        assertEquals(
            InsightsPiePalettePreset.VIVID,
            repository.insightsPiePalettePreset.first()
        )
    }

    @Test
    fun setInsightsPiePalettePreset_persistsSelection() = runTest {
        val repository = buildRepository()

        repository.setInsightsPiePalettePreset(InsightsPiePalettePreset.EDITORIAL)

        assertEquals(
            InsightsPiePalettePreset.EDITORIAL,
            repository.insightsPiePalettePreset.first()
        )
    }

    @Test
    fun insightsComparisonColorScheme_defaultsAndPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(
            InsightsComparisonColorScheme.GREEN_RED,
            repository.insightsComparisonColorScheme.first()
        )

        repository.setInsightsComparisonColorScheme(InsightsComparisonColorScheme.BLUE_ORANGE)

        assertEquals(
            InsightsComparisonColorScheme.BLUE_ORANGE,
            repository.insightsComparisonColorScheme.first()
        )
    }

    @Test
    fun insightsComparisonIndicatorStyle_defaultsAndPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(
            InsightsComparisonIndicatorStyle.ARROWS,
            repository.insightsComparisonIndicatorStyle.first()
        )

        repository.setInsightsComparisonIndicatorStyle(InsightsComparisonIndicatorStyle.TREND_LINES)

        assertEquals(
            InsightsComparisonIndicatorStyle.TREND_LINES,
            repository.insightsComparisonIndicatorStyle.first()
        )
    }

    @Test
    fun setRecordLastAuthoringMode_persistsSelection() = runTest {
        val repository = buildRepository()

        repository.setRecordLastAuthoringMode(RecordAuthoringMode.INTERVAL)

        assertEquals(
            RecordAuthoringMode.INTERVAL,
            repository.recordPersistedInput.first().lastAuthoringMode
        )
    }

    @Test
    fun setRecordLastTxtOutputMode_persistsSelection() = runTest {
        val repository = buildRepository()

        repository.setRecordLastTxtOutputMode(TxtOutputMode.DAY)

        assertEquals(
            TxtOutputMode.DAY,
            repository.recordPersistedInput.first().lastTxtOutputMode
        )
    }

    @Test
    fun recordPersistedInput_defaultsToInterval_whenModeHasNotBeenSelected() = runTest {
        val repository = buildRepository()

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
        val repository = buildRepository()

        repository.setRecordFrequentOutputMode(RecordFrequentOutputMode.ALIAS)

        assertEquals(
            RecordFrequentOutputMode.ALIAS,
            repository.recordFrequentPreferences.first().outputMode
        )
    }

    @Test
    fun insightsChartSemanticMode_defaultsToBreakdown_andPersistsSelection() = runTest {
        val repository = buildRepository()

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
        val repository = buildRepository()

        repository.setInsightsHeatmapPaletteName("BLUE_LIGHT")

        assertEquals("BLUE_LIGHT", repository.insightsHeatmapPaletteName.first())
    }

    @Test
    fun statusConfigs_areStoredIndependentlyForEachInsightsMode() = runTest {
        val repository = buildRepository()
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
        val repository = buildRepository()

        assertEquals("", repository.insightsChartTrendRoot.first())

        repository.setInsightsChartTrendRoot(" study ")

        assertEquals("study", repository.insightsChartTrendRoot.first())
    }

    @Test
    fun insightsResultDisplayMode_isStoredIndependentlyForEachInsightsMode() = runTest {
        val repository = buildRepository()

        assertEquals(InsightsResultDisplayMode.DETAILS, repository.insightsResultDisplayMode.first())

        repository.setInsightsResultDisplayMode(InsightsResultDisplayMode.CHART)

        assertEquals(InsightsResultDisplayMode.CHART, repository.insightsResultDisplayMode.first())

        repository.setInsightsMode(InsightsMode.MONTH)
        assertEquals(InsightsResultDisplayMode.DETAILS, repository.insightsResultDisplayMode.first())

        repository.setInsightsResultDisplayMode(InsightsResultDisplayMode.CHART)
        repository.setInsightsMode(InsightsMode.DAY)

        assertEquals(InsightsResultDisplayMode.CHART, repository.insightsResultDisplayMode.first())
    }

    @Test
    fun insightsParameterSection_defaultsToDay_andPersistsSelection() = runTest {
        val repository = buildRepository()

        assertEquals(InsightsParameterSection.DAY, repository.insightsParameterSection.first())

        repository.setInsightsParameterSection(InsightsParameterSection.ACTIVITIES)

        assertEquals(InsightsParameterSection.ACTIVITIES, repository.insightsParameterSection.first())
    }

    @Test
    fun setRecordCanonicalCatalogSource_persistsSelection() = runTest {
        val repository = buildRepository()

        repository.setRecordCanonicalCatalogSource(CanonicalCatalogSource.CATEGORIES)

        assertEquals(
            CanonicalCatalogSource.CATEGORIES,
            repository.recordFrequentPreferences.first().canonicalCatalogSource
        )
    }

    @Test
    fun insightsActivitiesView_defaultsAndPersistsDayAndPeriodSelectionsIndependently() = runTest {
        val repository = buildRepository()

        assertEquals(InsightsActivityView.RECORDS, repository.insightsDayActivitiesView.first())
        assertEquals(InsightsActivityView.OVERVIEW, repository.insightsPeriodActivitiesView.first())

        repository.setInsightsDayActivitiesView(InsightsActivityView.OVERVIEW)
        repository.setInsightsPeriodActivitiesView(InsightsActivityView.RECORDS)

        assertEquals(InsightsActivityView.OVERVIEW, repository.insightsDayActivitiesView.first())
        assertEquals(InsightsActivityView.RECORDS, repository.insightsPeriodActivitiesView.first())
    }

    @Test
    fun insightsTimeParametersExpanded_defaultsExpanded_andPersistsCollapsedState() = runTest {
        val repository = buildRepository()

        assertEquals(true, repository.insightsTimeParametersExpanded.first())

        repository.setInsightsTimeParametersExpanded(false)

        assertEquals(false, repository.insightsTimeParametersExpanded.first())
    }

    @Test
    fun recordQuickAccessCardExpanded_defaultsExpanded_andPersistsCollapsedState() = runTest {
        val repository = buildRepository()

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
        val repository = buildRepository()

        assertEquals(
            true,
            repository.configCardExpansionPreferences.first().isExpanded(ConfigCard.APPEARANCE)
        )

        repository.setConfigCardExpanded(ConfigCard.APPEARANCE, false)
        repository.setConfigCardExpanded(ConfigCard.DATA_MANAGEMENT, false)
        repository.setConfigCardExpanded(ConfigCard.THEME_PALETTE, true)
        repository.setConfigCardExpanded(ConfigCard.INSIGHTS_CHART_STYLE, true)
        repository.setConfigCardExpanded(ConfigCard.INSIGHTS_COMPARISON, true)

        val preferences = repository.configCardExpansionPreferences.first()
        assertEquals(false, preferences.appearanceExpanded)
        assertEquals(false, preferences.dataManagementExpanded)
        assertEquals(true, preferences.applicationPreferencesExpanded)
        assertEquals(true, preferences.insightsSettingsExpanded)
        assertEquals(true, preferences.aboutExpanded)
        assertEquals(true, preferences.themePaletteExpanded)
        assertEquals(true, preferences.insightsChartStyleExpanded)
        assertEquals(true, preferences.insightsComparisonExpanded)
    }

    @Test
    fun setRecordCanonicalCatalogDisplayMode_persistsSelection() = runTest {
        val repository = buildRepository()

        repository.setRecordCanonicalCatalogDisplayMode(RecordFrequentOutputMode.ALIAS)

        assertEquals(
            RecordFrequentOutputMode.ALIAS,
            repository.recordFrequentPreferences.first().canonicalCatalogDisplayMode
        )
    }

    @Test
    fun recordFrequentPreferences_defaultsCollapsedCanonicalRootPathsToEmpty() = runTest {
        val repository = buildRepository()

        assertEquals(
            emptySet<String>(),
            repository.recordFrequentPreferences.first().collapsedCanonicalRootPaths
        )
    }

    @Test
    fun setRecordCollapsedCanonicalRootPaths_persistsNormalizedRoots() = runTest {
        val repository = buildRepository()

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
        val repository = buildRepository()

        assertEquals(
            emptyList<String>(),
            repository.recordFrequentPreferences.first().orderedCanonicalRootPaths
        )
    }

    @Test
    fun setRecordOrderedCanonicalRootPaths_persistsNormalizedOrder() = runTest {
        val repository = buildRepository()

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
        val repository = buildRepository()

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
        val repository = buildRepository()

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

    private fun buildRepository(): UserPreferencesRepository {
        return UserPreferencesRepository(InMemoryPreferencesDataStore())
    }
}

private class InMemoryPreferencesDataStore : DataStore<Preferences> {
    private val state = MutableStateFlow<Preferences>(emptyPreferences())
    private val mutex = Mutex()

    override val data: Flow<Preferences> = state

    override suspend fun updateData(transform: suspend (Preferences) -> Preferences): Preferences =
        mutex.withLock {
            transform(state.value).also { state.value = it }
        }
}
